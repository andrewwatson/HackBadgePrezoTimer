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

//***** these modes are used to change state from green to yellow and to red as time runs out
#define MODE_INACTIVE 0
#define MODE_RUNNING 1
#define MODE_WARNING 2
#define MODE_SHUTOFF 3

#define DURATION_RUNNING 5000
#define DURATION_WARNING 3000
#define DURATION_SHUTOFF 3000

int currentMode;

void setup() {

    grid.begin();
    delay(50);
    grid.setBrightness(100);
    grid.clear();
    grid.show();

    Serial.begin(115200);
    
    currentMode = MODE_INACTIVE;
    delay(25);
    Particle.function("start", startTimer);
    Particle.function("stop", stopTimer);
}

int startTimer(String command) {

    currentMode = MODE_RUNNING;
    state_transition();
    return 0;
}

int stopTimer(String command) {
  currentMode = MODE_INACTIVE;
  state_transition();
  return 0;
}

void updateDisplay() {
    
    switch (currentMode) {
        case MODE_RUNNING:
            play_sequence(200, snake_animation_lut);
        break;
        
    }
}

void play_sequence(uint8_t framerate, uint32_t sequence[][16]) {

  uint16_t animation_length = (sizeof(sequence[0]) / sizeof(sequence));    //The number of frames in the above array

  // Particle.publish("debug", String::format("Length %d %d %d", sizeof(sequence) , sizeof(sequence[0]), animation_length));
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
    // missed_call = false;
    // rain_flashes++;
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

void loop() {
    updateDisplay();
}
