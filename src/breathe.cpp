#include <Arduino.h>
#include <JC_Button.h>
#include <TaskScheduler.h>

// Elk pootje van de Piezzo op 1 pin zetten. Door 1 hoog en ander laag te zetten creeer je meer 
// spanningsverschil en dus meer geluid.
#define SOUND_PIN 6
#define SOUND_PIN2 7

#define BREATHE_PIN 4

void beep();
void detectBreath();
long calculate_bpms();

Scheduler _runner;
Task _taskBeep( 500 * TASK_MILLISECOND, 3, &beep);
Task _taskDetectBreath( 50 * TASK_MILLISECOND, TASK_FOREVER, &detectBreath);

Button breathDetector(BREATHE_PIN);

long lastReminder = millis();

const int taps_len = 24;
unsigned long taps_millis[taps_len];
long next_tap = 0;
long total_taps = 0;
long time_signiture = 4;
long bpm  = 0;
#define SIXTY_S 60000

void setup() {
    Serial.begin(9600);

    pinMode(SOUND_PIN, OUTPUT);
    pinMode(SOUND_PIN2, OUTPUT);
    
    pinMode(LED_BUILTIN, OUTPUT);
    breathDetector.begin();

    _runner.addTask(_taskBeep);
    _taskBeep.enable();
    _runner.addTask(_taskDetectBreath);
    _taskDetectBreath.enable();
}

void loop() {
    _runner.execute();
}

void detectBreath() {
    breathDetector.read();

    if(breathDetector.wasReleased()) {
        taps_millis[next_tap] = millis();
        Serial.println(String(next_tap) + ": " + String(taps_millis[next_tap]));
        bpm  = calculate_bpms();
        next_tap = ++next_tap % taps_len;
        total_taps++;
        Serial.println("BPM: " + String(bpm));
        digitalWrite(LED_BUILTIN,HIGH);
        digitalWrite(LED_BUILTIN,LOW);
    }

    if( bpm > 20 && millis() - lastReminder > 5000) {
        _taskBeep.setInterval(500);
        _taskBeep.setIterations(3);
        _taskBeep.enableIfNot();
        lastReminder = millis();
        Serial.println("Reminder: Slow down");
    }

    if( bpm < 10 && millis() - lastReminder > 5000) {
        _taskBeep.setInterval(300);
        _taskBeep.setIterations(3);
        _taskBeep.enableIfNot();
        lastReminder = millis();
        Serial.println("Reminder: Speed up");
    }
}

void beep() {
    digitalWrite(SOUND_PIN2, LOW);
    tone(SOUND_PIN, 4000); // Send 1KHz sound signal...
    delay(1);
    noTone(SOUND_PIN);     // Stop sound...
    digitalWrite(SOUND_PIN2, HIGH);
}

// From: https://github.com/victorman/ArduinoMetronome/blob/master/metronome/metronome.ino
long calculate_bpms() {
  if(total_taps < time_signiture) {
    return 0;
  }
  
  unsigned long total = 0;
  for(int i=1; i<=time_signiture; i++) {
    int tap = next_tap - i;
    
    if(tap < 0) {
      tap = taps_len + tap;
    }
    
    if(i>1) {
      Serial.print(String(tap + 1 % taps_len) + ": " + String(taps_millis[tap+1%taps_len]) + " - ");
      Serial.print(String(tap) + ": " + String(taps_millis[tap]));
      
      total += (taps_millis[tap + 1 % taps_len] - taps_millis[tap]);
    }
    Serial.println(total);
  }
  
  return SIXTY_S/(total / (time_signiture - 1));
}
