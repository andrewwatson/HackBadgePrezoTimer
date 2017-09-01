// This #include statement was automatically added by the Particle IDE.
#include <neopixel.h>

#include <stdint.h>

#include "sequences.h"

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 16
#define PIXEL_PIN A5
#define PIXEL_TYPE WS2812B
#define MAX_BRIGHTNESS 200

uint8_t grid_brightness = 200;
uint8_t state_transition_flag = 1;
uint8_t mode_transition_flag = 1;

Adafruit_NeoPixel grid(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);

int currentPixel = 1;

uint32_t red = grid.Color(255, 0, 0);
uint32_t green = grid.Color(0, 255, 0);
uint32_t blue = grid.Color(0, 0, 255);
uint32_t white = grid.Color(255, 255, 255);
uint32_t magenta = grid.Color(255, 0, 255);
uint32_t yellow = grid.Color(255,255,0);
uint32_t black = grid.Color(0,0,0);

// uint16_t custom_animation_framerate = 200;

//***************************
//Animation control variables
static uint16_t frame = 0;
static uint32_t begin = 0;

const uint8_t pixel_map[] = {
  0,7,8,15,
  1,6,9,14,
  2,5,10,13,
  3,4,11,12
};

const uint8_t chase_map[] = {
  0, 7, 8, 15,
  14, 13, 12, 11,
  4, 3, 2, 1,
  6, 9, 10, 5
};

//***** these modes are used to change state from green to yellow and to red as time runs out
#define MODE_INACTIVE 0
#define MODE_RUNNING 1
#define MODE_WARNING 2
#define MODE_SHUTOFF 3

#define DURATION_RUNNING 5000
#define DURATION_WARNING 3000
#define DURATION_SHUTOFF 3000

uint32_t currentColor = black;

int currentMode;
int currentLevel;
int startTime;
int lastCheck;

const int timerDuration = 8 * 60 * 1000;
const uint8_t timerBrightness = 60;
const double warningThreshold = 0.80;
const double cutoffThreshold = 0.95;

void setup() {

  grid.begin();
  delay(50);
  grid.setBrightness(timerBrightness);
  grid.clear();
  grid.show();

  Serial.begin(115200);
  
  currentMode = MODE_INACTIVE;
  delay(25);
  Particle.function("start", startTimer);
  Particle.function("stop", stopTimer);

  currentLevel = 0;
  Particle.publish("timer", "ONLINE");
  fill_worm(red);
  fill_worm(green);
  fill_worm(magenta);
  fill_worm(yellow);
  fill_worm(black);
}

int startTimer(String command) {
  currentMode = MODE_RUNNING;
  currentLevel = 0;    
  state_transition();

  fill_worm(green);
  Particle.publish("timer", "ENTER RUNNING STATE");
  startTime = millis();
  lastCheck = startTime;

  return 0;
}

int stopTimer(String command) {
  currentMode = MODE_INACTIVE;
  state_transition();
  fill_worm(white);
  fill_worm(black);
  return 0;
}

void updateDisplay() {

  if (currentMode != MODE_INACTIVE) {
    int currentTime = millis();
    int duration = currentTime - startTime;        
    float percentComplete = ((float)duration / (float)timerDuration );

    if (currentTime - lastCheck > 60000) {
      lastCheck = millis();
      uint32_t tickPixel = grid.getPixelColor(0);
      set_all_pixels(white, timerBrightness);
      delay(150);
      set_all_pixels(tickPixel, timerBrightness);

      Particle.publish("timer", String::format("STATE %d D %ld PERCENT %0.2f", currentMode, duration, percentComplete));
    }

    switch (currentMode) {
      case MODE_RUNNING:
        // set_all_pixels(green, timerBrightness);
        if (percentComplete > warningThreshold) {
          currentMode = MODE_WARNING;
          Particle.publish("timer", String::format("ENTER %s STATE %ld %0.2f", "WARNING", duration, percentComplete));
          fill_worm(yellow);
        }
        break;

      case MODE_WARNING:
        if (percentComplete > cutoffThreshold) {
          currentMode = MODE_SHUTOFF;
          Particle.publish("timer", String::format("ENTER %s STATE %ld %0.2f", "SHUTDOWN", duration, percentComplete));
          fill_worm(red);
        }
        set_all_pixels(yellow, timerBrightness);
        break;
      case MODE_SHUTOFF:
        fill_worm(black);
        fill_worm(red);
        delay(500);
        // set_all_pixels(red, timerBrightness);
    }
  }
}

void play_sequence(uint8_t framerate, uint32_t sequence[][16], uint16_t animation_length) {

  //Check if enough time has passed between frames
  if(time_since(begin) < framerate)
  {
    return;
  }

  //*****************************************
  //Check if we're just starting the sequence
  if(state_transition_flag)
  {
    grid.setBrightness(100);
    state_transition_flag = 0;
    frame = 0;
  }
  begin = millis();

  //*****************************************
  //Update the LEDs to display the next frame
  for(uint8_t i=0; i<grid.numPixels(); i++)
  {
    grid.setPixelColor(pixel_map[i],sequence[frame][i]);
  }
  grid.show();

  //*********************************************************************
  //Set the animation to loop by starting from the beginning once it ends
  if(++frame >= animation_length)
  {
    frame = 0;
  }

  return;
}

//**************************************************
//Return the number of milliseconds since start_time
uint32_t time_since(uint32_t start_time)
{
  return(millis()-start_time);
}

//********************************************************************
//Clear the display and set the flag if a state transition has occured
void state_transition(void)
{
  set_all_pixels(black, 0);
  delay(10);                    //TODO see if we can reduce this delay
  state_transition_flag = 1;
}

//******************************************
//Set the color and brightness of all pixels
void set_all_pixels(uint32_t c, uint8_t brightness)
{
  uint8_t i;

  //***************************
  //Set the brightness globally
  if(brightness>0)
  {
    if(brightness < MAX_BRIGHTNESS)
    {
      grid.setBrightness(brightness);
    }
    else
    {
      grid.setBrightness(MAX_BRIGHTNESS);
    }
  }

  //*********************************
  //Set each pixel color individually
  for(i=0; i<grid.numPixels(); i++)
  {
    grid.setPixelColor(i, c);
  }

  grid.show();
}

// fills the grid with a new color, 1 pixel at a time
void fill_worm(uint32_t c) {
  
  for (int i = 0; i < PIXEL_COUNT; i++) {
    grid.setPixelColor(chase_map[i], c);
    grid.show();
    delay(25);
  }
  
}

void loop() {
    updateDisplay();
}
