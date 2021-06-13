/*===========================================================
 * Model Stand prototype code.
 * 2 LEDs
 * Driven by Tiny85, but with Uno support for debugging.   
 */


#if defined(__AVR_ATmega328P__)
#define PLATFORM_UNO
#elif defined(__AVR_ATtiny85__)
#define PLATFORM_TINY
#else
#error "PLATFORM UNSUPPORTED"
#endif

#ifdef PLATFORM_UNO
#define DEBUG_PRINT(item) Serial.print(item)
#define DEBUG_PRINTLN(item) Serial.println(item)
#else
#define DEBUG_PRINT(item)
#define DEBUG_PRINTLN(item)
#endif


#include <Adafruit_NeoPixel.h>

// Which pin to use for DualRingLED control
#define LED_PIN    4  // ATTINY pin 3 is D4
#define POT_PIN    A1 // ATTINY pin 7 is A1
#define NUMPIXELS  2 

#ifdef PLATFORM_TINY
#define BUTTON_PIN 0  // ATTINY pin 5 is D0
#else
#define BUTTON_PIN 8  // Can't use D0 on Uno...reserved for serial.
#endif
#define DEBOUNCE_MS 50

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB+NEO_KHZ800);

#define COLOR_RED     0xFF0000
#define COLOR_GREEN   0x00FF00
#define COLOR_BLUE    0x0000FF
#define COLOR_MAGENTA 0xFF00FF
#define COLOR_YELLOW  0xFFFF00
#define COLOR_CYAN    0x00FFFF
#define COLOR_BLACK   0
#define COLOR_WHITE   0xFFFFFF

typedef enum 
{
  MODE_SET_COLOR,
  MODE_SET_BRIGHTNESS,
  MODE_BREATHE,
  MODE_RAINBOW,
  NUM_MODES
} mode_type;

void init_set_color(void);
void process_set_color(void);
void init_set_brightness(void);
void process_set_brightness(void);
void init_breathe(void);
void process_breathe(void);
void init_rainbow(void);
void process_rainbow(void);

typedef void (*init_func_type)(void);
typedef void (*process_func_type)(void);

init_func_type init_table[] = 
{
  init_set_color,
  init_set_brightness,
  init_breathe,
  init_rainbow
};

process_func_type processing_table[] =
{
  process_set_color,
  process_set_brightness,
  process_breathe,
  process_rainbow
};

int current_mode=0;

uint32_t update_rate_ms = 1;
uint32_t last_update_ms=0;

int max_brightness=1023;  // 1023 = full brightness.  0 = off.
int current_brightness=1023;

uint32_t current_color=COLOR_RED;

// this function takes a 24-bit CRGB color and returns a color 
// scaled down by the global brightness factor.
uint32_t scale_brightness(uint32_t color)
{
  uint32_t scaled_brightness=0;
  uint32_t  r_byte;
  uint32_t  g_byte;
  uint32_t  b_byte;
  
  //brightness is going to map to a percentage for us to apply to each 8 bits of the RGB value.
  // 1023 will map to full (divide by 1)
  // 0 will map to off (set to 0)
  // for anything in between, we'll use a fraction...so 512 will be half brightness.

  // start with the B byte...easiest
  b_byte = color & 0x0000FF;
  b_byte = b_byte * current_brightness / 1023;

  g_byte = color & 0x00FF00;
  g_byte = g_byte >> 8;
  g_byte = g_byte * current_brightness / 1023;

  r_byte = color & 0xFF0000;
  r_byte = r_byte >> 16;
  r_byte = r_byte * current_brightness / 1023;

  // Now that we have the individual bytes, build back the composite color.
  scaled_brightness = r_byte << 16;
  scaled_brightness = scaled_brightness | (g_byte << 8);
  scaled_brightness = scaled_brightness | b_byte;
  
  return (scaled_brightness);
}

void init_set_color(void)
{
  DEBUG_PRINTLN("init set color");
  
  // revert to max brightness in case we came from some "breathing" scenario
  current_brightness = max_brightness;
  update_rate_ms = 1;  
}

#define BLUE_START 100
#define GREEN_START (BLUE_START + 255)
#define RED_START   (GREEN_START + 255)
#define WHITE_START (RED_START + 255)
void process_set_color(void)
{
  int      pot_value;
  int i;
  int  r_value=0;
  int  g_value=0;
  int  b_value=0;

  
  pot_value = analogRead(POT_PIN);
  // 0-99 is "off"
  // 100-355 goes from Blue to Green
  // 356 to 612 goes from Green to Red
  // 612 to 868 goes from Red to Blue
  // 868 and higher is white

  #if 0
  DEBUG_PRINT("Pot: ");
  DEBUG_PRINT(pot_value);
  #endif
  
  // black (off) for the low range
  if (pot_value < BLUE_START)
  {
    // already zeroed out due to inits.
  }
  // between blue and green
  else if (pot_value < GREEN_START)
  {
    g_value = pot_value - BLUE_START;
    b_value = 255 - g_value;
  }
  // between green and red
  else if (pot_value < RED_START)
  {
    r_value = pot_value - GREEN_START;
    g_value = 255 - r_value;
  }
  else if (pot_value < WHITE_START)
  {
    b_value = pot_value - RED_START;
    r_value = 255 - b_value;
  }
  else
  {
    r_value = 255;
    g_value = 255;
    b_value = 255;
  }

  #if 0
  DEBUG_PRINT(" R:");
  DEBUG_PRINT(r_value);
  DEBUG_PRINT(" G:");
  DEBUG_PRINT(g_value);
  DEBUG_PRINT(" B:");
  DEBUG_PRINTLN(b_value);
  #endif
  
  // set all LEDs to the proper color.
  for (i=0; i<NUMPIXELS; i++)
  {
    // currently ignoring brightness...
    pixels.setPixelColor(i, r_value, g_value, b_value);
  }
  pixels.show();

  // and gonna want this in "current color"...
  current_color = b_value;
  current_color |= (g_value << 8);
  current_color |= (r_value << 16);
}

void init_set_brightness(void)
{
  DEBUG_PRINTLN("init set brightness");
  update_rate_ms = 10;

}

void process_set_brightness(void)
{ 
  int pot_value;
  int i;
  
  /* read the knob */
  pot_value = analogRead(POT_PIN);
  max_brightness = pot_value;
  current_brightness = pot_value;
  
  // scale our current color by our brightness value.
  for (i=0; i<NUMPIXELS; i++)
  {
    pixels.setPixelColor(i,scale_brightness(current_color));
  }
  pixels.show();
  
  
  
}

void init_breathe(void)
{
  DEBUG_PRINT("Init breathe");
  update_rate_ms = 10;
  current_brightness = max_brightness;
}

void process_breathe(void)
{
  static int dir=-1;  // -1 means down, +1 means up.
  int i;
  int step_size;

  // eventually want pot reads to be speed...but not just yet.


  if      (current_brightness < 128)  step_size = 1;
  else if (current_brightness < 256)  step_size = 2;
  else if (current_brightness < 512)  step_size = 4;
  else                                step_size = 8;
  
  // walk our current brightness down or up.
  current_brightness += dir*step_size;

  // if we've hit either the top or bottom of our range, reverse direction
  if ((current_brightness <= 0) || (current_brightness >= max_brightness))
  {
    // DEBUG_PRINTLN("Reversing direction");
    
    dir = dir * -1;
  }


  // finally, show all the pixels at our current brightness.
  for (i=0; i<NUMPIXELS; i++)
  {
    pixels.setPixelColor(i,scale_brightness(current_color));
  }
  pixels.show();
}

// Fun UI thought:  we set brightness based on the dial before we enter this mode...
// then we use the dial to adjust speed!!

void init_rainbow( void )
{
  DEBUG_PRINT("init rainbow");
}

void process_rainbow( void )
{
  

}

void setup()
{
    int i;
    
    #ifdef PLATFORM_UNO
    Serial.begin(9600);
    #endif
    
    pixels.begin();
    // Power on self-test...see if all the pixels are working.
    for (i=0; i<NUMPIXELS; i++)
    {
      pixels.setPixelColor(i,COLOR_GREEN);
      delay(50);
      pixels.show();
    }

    delay(1000);

    #ifdef PLATFORM_UNO
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    #else
    pinMode(BUTTON_PIN, INPUT);
    #endif

    init_set_color();

}

void loop()
{
  uint32_t        current_ms;
  static uint32_t press_ms=0;   // use this for debouncing.
  static int      last_button_state = HIGH;
  int             current_button_state;
  
  // check for button presses to switch modes.
  current_button_state = digitalRead(BUTTON_PIN);
  if ((current_button_state == HIGH) && (last_button_state == LOW))
  {
    // check timestamp on releases
    current_ms = millis();
    if (current_ms > press_ms + DEBOUNCE_MS)
    {
      // this was a legit release.  Tweak state
      DEBUG_PRINT("PRESS:  ");
      DEBUG_PRINT(current_mode);
      current_mode = (current_mode + 1) % NUM_MODES; // This was %4...trying without magic #
      init_table[current_mode]();
    }
  }
  else if ((current_button_state == LOW) && (last_button_state == HIGH))
  {
    // Start of a press.  Mark the timestamp.
    press_ms = millis();
  }
  last_button_state = current_button_state;

  // call appropriate processing function
  current_ms = millis();
  if (current_ms > last_update_ms + update_rate_ms)
  {
    last_update_ms = current_ms;
    processing_table[current_mode]();
  }
}
