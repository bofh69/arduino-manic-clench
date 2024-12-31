#include <LiquidCrystal.h>
#include "pitches.h"

struct GameData {
  int sensorValueX;
  int sensorValueY;
  int middleX;
  int middleY;

  bool playing;

  int left_wall;
  int width;

  int player_val;
  int player_pos;

  int score;
  int hiscore;
};


// The display also needs some more signals, see the LCD example
// in Elegoo's documentation.
static const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 7;
static LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

static const int sensorPinY = A1;
static const int sensorPinX = A0;
static const int ledPin = LED_BUILTIN;
static const int switchPin = 6;
static const int speakerPin = 2;

static GameData gamedata;

///////////////////////////////
// From the music sketch:

static const int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};
// note durations: 4 = quarter note, 8 = eighth note, etc.:
static const int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};

static void play_music() {
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(speakerPin, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(speakerPin);
  }
}

///////////////////////////////

static void initLcd() {
  lcd.begin(16, 2);
  // lcd.autoscroll();
  // Print a message to the LCD.
  lcd.print("Hello friend.");
}

static void calibrateJoystick() {
  pinMode(switchPin, INPUT_PULLUP);
  gamedata.middleX = analogRead(sensorPinX);
  gamedata.middleY = analogRead(sensorPinY);
}

static bool switch_get() {
  return !digitalRead(switchPin);
}

void setup() {
  randomSeed(0);

  pinMode(ledPin, OUTPUT);
  initLcd();
  calibrateJoystick();
}

void loop() {
  char buff[100];

  if (gamedata.playing) {

    gamedata.sensorValueX = analogRead(sensorPinX) - gamedata.middleX;

    switch (random(0, 3)) {
      case 0:
        if (gamedata.left_wall > 0) gamedata.left_wall -= 1;
        break;
      case 1:
        if ((gamedata.left_wall + gamedata.width) < 14) gamedata.left_wall += 1;
        break;
    }

    gamedata.player_val += gamedata.sensorValueX / 3;
    gamedata.player_pos = gamedata.player_val / 100;

    if (gamedata.player_pos <= gamedata.left_wall) {
      gamedata.playing = false;
    } else if (gamedata.player_pos >= (gamedata.left_wall + gamedata.width)) {
      gamedata.playing = false;
    } else {
      gamedata.score++;
    }

    lcd.setCursor(0, 0);
    sprintf(buff, "Score: %-9d", gamedata.score);
    lcd.print(buff);

    memset(buff, ' ', 15);
    buff[gamedata.left_wall] = '|';
    buff[gamedata.player_pos] = 'V';
    buff[gamedata.left_wall + gamedata.width] = '|';
    buff[15] = 0;
    lcd.setCursor(0, 1);
    lcd.print(buff);

    delay(100);
  } else {
    if (gamedata.width > 0) {
      // ! first time.
      lcd.setCursor(gamedata.player_pos, 1);
      lcd.print('*');

      if (gamedata.score > gamedata.hiscore) {
        gamedata.hiscore = gamedata.score;
      }

      lcd.setCursor(0, 0);
      sprintf(buff, "Score: %d/%-10d", gamedata.score, gamedata.hiscore);
      lcd.print(buff);

      play_music();
    }

    // Wait for button press to continue:
    while (!switch_get()) {
      delay(1);
    }

    gamedata.left_wall = 3;
    gamedata.width = 7;
    gamedata.player_pos = gamedata.left_wall + gamedata.width / 2;
    gamedata.player_val = gamedata.player_pos * 100;
    gamedata.score = 0;
    gamedata.playing = true;
    lcd.clear();
  }
}
