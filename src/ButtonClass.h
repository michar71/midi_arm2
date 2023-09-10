#ifndef ButtonClass_h
#define ButtonClaass_h

#include <math.h>
#include "Arduino.h"

#define LONG_PRESS_MS 800
#define VERY_LONG_PRESS_MS 2000
#define VERY_VERY_LONG_PRESS_MS 5000
#define TOUCH_THRESHOLD 40

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

    uint16_t long_press_ms = LONG_PRESS_MS;
    uint16_t very_long_press_ms = VERY_LONG_PRESS_MS;
    uint16_t very_very_long_press_ms = VERY_VERY_LONG_PRESS_MS;
    uint16_t touch_th = TOUCH_THRESHOLD;
public:
    ButtonClass(uint8_t pin, bool isTouch = false);
    mode_button_e check_button(void);
    void setTiming(uint16_t long_press_ms,uint16_t very_long_press_ms,uint16_t very_very_long_press_ms);
    void setTouchThreshold(uint16_t touch_th);


};



#endif //ButtonClass_h