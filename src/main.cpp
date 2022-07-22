#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include "Arduino.h"


#define NOTE_C3 131
// Constants
#define NUM_JOY 2
#define MIN_VAL 0
#define MAX_VAL 1023
// Parameters
const int joyPin[2] = {33, 32};
const int joyBtn = 2;
const int joyOffset = 0;
const int buzPin = 13;
const int btnPin = 26;

const int timeLed1 = 27;
const int timeLed2 = 14;
const int timeLed3 = 25;


// Variables
int btnVal = 0;
int joyVal[NUM_JOY] = {0, 0};

int numberOfHorizontalDisplays = 1;
int numberOfVerticalDisplays = 1;
int pinCS = 5; // Attach CS to this pin, DIN to MOSI and CLK to SCK 

Max72xxPanel matrix = Max72xxPanel(pinCS, 1, 1);

const int pinRandom = A0;

const int wait = 133; // In milliseconds
const int length = 4;
int score = 0;

int x[length], y[length];
int xApple, yApple;
int ptr, nextPtr;
int prevDir = 0;

bool displayScore = false;
String scoreText = "Score ";
int scoreSpacer = 1;
int scoreWidth = 5 + scoreSpacer; // The font width is 5 pixels

bool displayEndGame = false;
const unsigned int endGame = 60000;
unsigned int timer = endGame;
String restartText = "Press button to restart ";
int restartSpacer = 1;
int restartWidth = 5 + restartSpacer; // The font width is 5 pixels

void detectScoreButton()
{
  btnVal = analogRead(btnPin);
  Serial.println(btnVal);
  if (btnVal == 0)
  {
    Serial.println(displayScore ? "DISPLAY": "NON");
    displayScore = true;
  }
}

void mountScore()
{
  scoreText =  String(score) + " ";
  for (int i = 0; i < scoreWidth * scoreText.length() + matrix.width() - 1 - scoreSpacer; i++)
  {

    matrix.fillScreen(LOW);

    int letter = i / scoreWidth;
    int x = (matrix.width() - 1) - i % scoreWidth;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while (x + scoreWidth - scoreSpacer >= 0 && letter >= 0)
    {
      if (letter < scoreText.length())
      {
        matrix.drawChar(x, y, scoreText[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= scoreWidth;
    }

    matrix.write(); // Send bitmap to display

    delay(wait);
    displayScore = false;
  }
}

void mountRestartMenu() {
  for (int i = 0; i < restartWidth * restartText.length() + matrix.width() - 1 - restartSpacer; i++)
  {

    matrix.fillScreen(LOW);

    int letter = i / restartWidth;
    int x = (matrix.width() - 1) - i % restartWidth;
    int y = (matrix.height() - 8) / 2; // center the text vertically

    while (x + restartWidth - restartSpacer >= 0 && letter >= 0)
    {
      if (letter < restartText.length())
      {
        matrix.drawChar(x, y, restartText[letter], HIGH, LOW, 1);
      }

      letter--;
      x -= restartWidth;
    }

    matrix.write(); // Send bitmap to display

    delay(80);
  }
}

float joyRawToPhys(int raw)
{ /* function joyRawToPhys */
  ////Joystick conversion rule
  float phys = map(raw, MIN_VAL, MAX_VAL, -100 + joyOffset, 100 + joyOffset) - joyOffset;
  return phys;
}

int readJoystick()
{ /* function readJoystick */
  ////Test routine for Joystick
  for (int i = 0; i < NUM_JOY; i++)
  {
    joyVal[i] = analogRead(joyPin[i]);
  }
  float yMove = joyRawToPhys(joyVal[1]);
  float xMove = joyRawToPhys(joyVal[0]);

  // Serial.println("xMove");
  // Serial.println(xMove);
  // Serial.println("yMove");
  // Serial.println(yMove);

  if (yMove >= 690)
  {
    prevDir = 1;
  }
  else if (yMove <= -90)
  {
    prevDir = 0;
  }
  else if (xMove <= -90)
  {
    prevDir = 3;
  }
  else if (xMove >= 690)
  {
    prevDir = 2;
  }
  // Serial.print(F("prevDir")), Serial.print(F(" : ")); Serial.println(prevDir);
  return prevDir;
}

boolean equal(int ptrA, int ptrB)
{
  return x[ptrA] == x[ptrB] && y[ptrA] == y[ptrB];
}

boolean occupied(int ptrA)
{
  for (int ptrB = 0; ptrB < length; ptrB++)
  {
    if (ptrA != ptrB)
    {
      if (equal(ptrA, ptrB))
      {
        return true;
      }
    }
  }

  return false;
}

int next(int ptr)
{
  return (ptr + 1) % length;
}

void spawnSnake() {
  for (ptr = 0; ptr < length; ptr++)
  {
    x[ptr] = numberOfHorizontalDisplays * 8 / 2;
    y[ptr] = numberOfVerticalDisplays * 8 / 2;
  }
  nextPtr = 0;
}

void spawnApple()
{
  xApple = random(8);
  yApple = random(8);
}

void checkCollisionWithApple()
{
  // Serial.print("x : ");
  // Serial.print(x[ptr]);
  // Serial.print("\n");
  // Serial.print("y : ");
  // Serial.print(y[ptr]);
  // Serial.print("\n");
  if (x[ptr] == xApple && y[ptr] == yApple)
  {
    score++;
    spawnApple();
    Serial.print(score);
    ledcAttachPin(buzPin, 0); // broche 18, canal 0.
    ledcSetup(0, 5000, 12); // canal = 0, frequence = 5000 Hz, resolution = 12 bits
    ledcWrite(0, 2048);   //  canal = 0, rapport cyclique = 2048
    delay(200);
    ledcWrite(0, 0);
  }
}

void onEndGame() {
  mountScore();
  mountRestartMenu();
}

void setup()
{
  matrix.setIntensity(4);
  // Reset all variables
  spawnSnake();

  randomSeed(analogRead(pinRandom)); // Initialize random generator

  // Init Serial USB
  Serial.begin(9600);
  Serial.println(F("Initialize System"));
  // Init Joystick
  for (int i = 0; i < NUM_JOY; i++)
    pinMode(joyPin[i], INPUT);

  spawnApple();

  pinMode(btnPin, INPUT_PULLUP);
  pinMode(timeLed1, OUTPUT);
  pinMode(timeLed2, OUTPUT);
  pinMode(timeLed3, OUTPUT);
  pinMode(buzPin, OUTPUT);
}

void loop()
{
  // Leds for timer
  digitalWrite(timeLed1, timer > 0 ? HIGH : LOW);
  digitalWrite(timeLed2, (endGame / 3) < timer ? HIGH : LOW );
  digitalWrite(timeLed3, ( endGame / 3 ) * 2 < timer ? HIGH : LOW);

  detectScoreButton();

  if(displayEndGame) {
    onEndGame();
    return;
  }

  // Score menu 
  if (displayScore)
  {
    mountScore();
    return;
  }

  // Shift pointer to the next segment
  ptr = nextPtr;
  nextPtr = next(ptr);

  matrix.drawPixel(x[ptr], y[ptr], HIGH); // Draw the head of the snake

  matrix.drawPixel(xApple, yApple, HIGH);

  matrix.write(); // Send bitmap to display

  checkCollisionWithApple();

  delay(wait);

  if(timer >= wait) {
    timer -= wait;
  }
  else {
    timer = 0;
    displayEndGame = true;
  }

  if (!occupied(nextPtr))
  {
    matrix.drawPixel(x[nextPtr], y[nextPtr], LOW); // Remove the tail of the snake
  }

  for (int attempt = 0; attempt < 10; attempt++)
  {

    // Jump at random one step up, down, left, or right
    switch (readJoystick())
    {
    case 0:
      x[nextPtr] = constrain(x[ptr] + 1, 0, matrix.width() - 1);
      y[nextPtr] = y[ptr];
      break;
    case 1:
      x[nextPtr] = constrain(x[ptr] - 1, 0, matrix.width() - 1);
      y[nextPtr] = y[ptr];
      break;
    case 2:
      y[nextPtr] = constrain(y[ptr] + 1, 0, matrix.height() - 1);
      x[nextPtr] = x[ptr];
      break;
    case 3:
      y[nextPtr] = constrain(y[ptr] - 1, 0, matrix.height() - 1);
      x[nextPtr] = x[ptr];
      break;
    }

    if (!occupied(nextPtr))
    {
      break; // The spot is empty, break out the for loop
    }
  }
}