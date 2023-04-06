//#define BUZZER
#define MIDI
#define TUNING_PITCH_HZ 440
#define TUNING_PITCH_MIDI_NOTE_NUM 69
#define BASE_MIDI_NOTE_NUM 31 // G1
#define SEMITONE_FACTOR 1.0594630943592953 // 2^(1/12)

#define ACTIVE_KEYBOARD_PINS_START 23
#define ACTIVE_KEYBOARD_PINS_NUM 6 // Half octave
#define ACTIVE_KEYBOARD_PINS_INCR 2

#define PASSIVE_KEYBOARD_PINS_START 22
#define PASSIVE_KEYBOARD_PINS_NUM 11
#define PASSIVE_KEYBOARD_PINS_INCR 2

#define PEDAL_PINS_START 35
#define PEDAL_PINS_NUM 1
#define PEDAL_PINS_INCR 2

#define BUZZER_PIN 12

bool last_key_state[ACTIVE_KEYBOARD_PINS_NUM][PASSIVE_KEYBOARD_PINS_NUM];
bool last_pedal_state[PEDAL_PINS_NUM];

#ifdef BUZZER
int keys_pressed;
#endif

void setup() {
#ifdef MIDI
  Serial.begin(31250);
#else
  Serial.begin(9600);
#endif
  for (uint8_t i = 0; i < ACTIVE_KEYBOARD_PINS_NUM; i++) {
    uint8_t a = ACTIVE_KEYBOARD_PINS_START + i * ACTIVE_KEYBOARD_PINS_INCR;
    pinMode(a, OUTPUT);
    digitalWrite(a, LOW);
  }
  for (uint8_t i = 0; i < PASSIVE_KEYBOARD_PINS_NUM; i++) {
    uint8_t p = PASSIVE_KEYBOARD_PINS_START + i * PASSIVE_KEYBOARD_PINS_INCR;
    pinMode(p, INPUT);
  }
  for (uint8_t i = 0; i < PEDAL_PINS_NUM; i++) {
    uint8_t p = PEDAL_PINS_START + i * PEDAL_PINS_INCR;
    pinMode(p, INPUT_PULLUP);
  }

  for (uint8_t i = 0; i < ACTIVE_KEYBOARD_PINS_NUM; i++) {
    for (uint8_t j = 0; j < PASSIVE_KEYBOARD_PINS_NUM; j++) {
      last_key_state[i][j] = false;
    }
  }
#ifdef BUZZER
  keys_pressed = 0;
#endif
#ifndef MIDI
  Serial.println("Initialized");
#endif
}

void loop() {
  for (uint8_t i = 0; i < ACTIVE_KEYBOARD_PINS_NUM; i++) {
    uint8_t a = ACTIVE_KEYBOARD_PINS_START + i * ACTIVE_KEYBOARD_PINS_INCR;
    digitalWrite(a, HIGH);
    delayMicroseconds(10);

    for (uint8_t j = 0; j < PASSIVE_KEYBOARD_PINS_NUM; j++) {
      uint8_t p = PASSIVE_KEYBOARD_PINS_START + j * PASSIVE_KEYBOARD_PINS_INCR;
      int sample = digitalRead(p);
      bool sampled_key_state = sample == HIGH;
      if (sampled_key_state != last_key_state[i][j]) {
        int midi_num = j * ACTIVE_KEYBOARD_PINS_NUM + i + BASE_MIDI_NOTE_NUM;
        if (sampled_key_state) {
#ifdef MIDI
          noteOn(0x90, midi_num, 0x45);
#else
          Serial.print("v ");
          Serial.print((int)i);
          Serial.print(' ');
          Serial.println((int)j);
#endif
#ifdef BUZZER
          keys_pressed += 1;
          tone(
            BUZZER_PIN,
            TUNING_PITCH_HZ * pow(
              SEMITONE_FACTOR,
              midi_num - TUNING_PITCH_MIDI_NOTE_NUM
            )
          );
#endif
        } else {
#ifdef MIDI
          noteOn(0x90, midi_num, 0x00);
#else
          Serial.print("^ ");
          Serial.print((int)i);
          Serial.print(' ');
          Serial.println((int)j);
#endif
#ifdef BUZZER
          keys_pressed -= 1;
          if (keys_pressed == 0) {
            noTone(BUZZER_PIN);
          }
#endif
        }
        last_key_state[i][j] = sampled_key_state;
      }
    }
    digitalWrite(a, LOW);
  }

  for (uint8_t i = 0; i < PEDAL_PINS_NUM; i++) {
    uint8_t p = PEDAL_PINS_START + i * PEDAL_PINS_INCR;
    int sample = digitalRead(p);
    bool sampled_pedal_state = sample == LOW;

    if (sampled_pedal_state != last_pedal_state[i]) {
#ifdef MIDI
      // TODO: Other pedals, sustain for buzzer
      noteOn(0xb0, 64, 0xff * sampled_pedal_state);
#endif
      last_pedal_state[i] = sampled_pedal_state;
    }
  }
}

#ifdef MIDI
void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}
#endif
