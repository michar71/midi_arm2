#include "ButtonClass.h" 
#include "Arduino.h"


ButtonClass::ButtonClass(uint8_t pin, bool isTouch)
{
    this->isTouch = isTouch;
    this->buttonID = pin;
}

mode_button_e ButtonClass::check_button(void)
{


    if (isTouch == false)
    {
        //New Button Press
        if ((LOW == digitalRead(buttonID)) && (this->buttonPressed == false))
        {
            this->buttonPressed = true;
            this->button_time = millis();  
            return DOWN; 
        }
        else if ((HIGH == digitalRead(buttonID)) && (this->buttonPressed == true))
        {
            this->buttonPressed = false;

            if ((millis() - this->button_time) > very_very_long_press_ms)
            {        
                return VERY_VERY_LONG_PRESS;
            }        
            else if ((millis() - this->button_time) > very_long_press_ms)
            {        
                return VERY_LONG_PRESS;
            }
            else if ((millis() - this->button_time) > long_press_ms)
            {         
                return LONG_PRESS;
            }
            else 
            {          
                return SHORT_PRESS;
            }
        }
        return NO_PRESS;
    }
    else
    {
        //New Button Press
        uint16_t tb = touchRead(this->buttonID);

        if ((tb > this->touch_th) && (this->buttonPressed == false) && (tb < this->touch_cutoff))
        {
            this->buttonPressed = true;
            this->button_time = millis();     
            return DOWN; 
        }
        else if ((tb < this->touch_th) && (this->buttonPressed == true) && (tb > (this->touch_th/100*60)))
        {
            this->buttonPressed = false;

            if ((millis() - this->button_time) > this->very_very_long_press_ms)
            {        
                return VERY_VERY_LONG_PRESS;
            }        
            else if ((millis() - this->button_time) >this-> very_long_press_ms)
            {        
                return VERY_LONG_PRESS;
            }
            else if ((millis() - this->button_time) > this->long_press_ms)
            {         
                return LONG_PRESS;
            }
            else 
            {            
                return SHORT_PRESS;
            }
        }
        return NO_PRESS;
    }
}

void ButtonClass::setTiming(uint16_t long_press_ms,uint16_t very_long_press_ms,uint16_t very_very_long_press_ms)
{
    this->long_press_ms = long_press_ms;
    this->very_long_press_ms = very_long_press_ms;
    this->very_very_long_press_ms = very_very_long_press_ms;
}

void ButtonClass::setTouchThreshold(uint16_t touch_th,uint16_t touch_cutoff)
{
    this->touch_th = touch_th;
    this->touch_cutoff = touch_cutoff;
}

void ButtonClass::setTouchMode(bool touch)
{
    this->isTouch = touch;
}