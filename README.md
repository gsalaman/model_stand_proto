# Model Stand Prototype
The purpose of this project is to see how 2 RGB LEDs can illuminate a translucent model.  I'll be using my Tiny85 control board, with button, pot, coin-cell, and power switch, and two individual WS2812 LEDs...one for each "foot" of the model.

## Modes
Pressing the button will cycle through the following modes:
* Use knob to set current color
* Use knob to set max brightness of current color
* "Pulse" current color (up to current max brightness)
* Cycle through all colors; knob sets cycle speed

```
typedef enum 
{
  MODE_SET_COLOR,
  MODE_SET_BRIGHTNESS,
  MODE_BREATHE,
  MODE_RAINBOW
  NUM_MODES
} mode_type;
```


