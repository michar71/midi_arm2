#ifndef ButtonClass_h
#define ButtonClaass_h

#include <math.h>
#include "Arduino.h"

#define LONG_PRESS_MS 800
#define VERY_LONG_PRESS_MS 2000
#define VERY_VERY_LONG_PRESS_MS 5000
#define TOUCH_THRESHOLD 40
#define TOUCH_CUTOFF 0xFFFF
    typedef enum{
        NO_PRESS,
        DOWN,
        SHORT_PRESS,
        LONG_PRESS,
        VERY_LONG_PRESS,
        VERY_VERY_LONG_PRESS
    }mode_button_e;

//--------------------------------------------------------------------------------------------
// Variable declaration

class ButtonClass {
//-------------------------------------------------------------------------------------------
// Function declarations



private:
    uint16_t buttonID;
    bool isTouch;
    bool buttonPressed = false;
    unsigned long button_time = 0;

    uint16_t long_press_ms = LONG_PRESS_MS;
    uint16_t very_long_press_ms = VERY_LONG_PRESS_MS;
    uint16_t very_very_long_press_ms = VERY_VERY_LONG_PRESS_MS;
    uint16_t min_touch = TOUCH_CUTOFF;
    uint16_t max_touch = 0;
    uint16_t touch_th = 0;
    bool autocal = false;
public:
    ButtonClass(uint8_t pin, bool isTouch = false,bool isAutoCalc = false);
    mode_button_e check_button(void);
    void setTiming(uint16_t long_press_ms,uint16_t very_long_press_ms,uint16_t very_very_long_press_ms);
    void setTouchThresholds(uint16_t touch_th_low,uint16_t touch_th_high);
    uint16_t getTouchThresholdHigh(void);
    uint16_t getTouchThresholdLow(void);
    uint16_t getTouchAnalog(uint16_t max);
    void setTouchMode(bool touch);
    void setAutocal(bool a);
};



#endif //ButtonClass_h