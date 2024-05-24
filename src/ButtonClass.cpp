#include "ButtonClass.h" 
#include "Arduino.h"

//Advanced auto Cal for touch

//Start with an inital Threshold (calibration?) Value
//Update Min/Max Over Time, put threshold in the middle
//Smooth out Min/Max by removing outliers through filter
//Allow to store/set min/max
//Q: Should we reset things at some point so they can use downwards again?
//Maybe rectec outliers at some point?

ButtonClass::ButtonClass(uint8_t pin, bool isTouch,bool isAutoCalc)
{
    this->isTouch = isTouch;
    this->buttonID = pin;
    this->autocal = isAutoCalc;
    this->touch_th =  (this->min_touch + this->max_touch) / 2;
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
#if BABOI_HW_VER == 2        
        uint16_t tb = touchRead(this->buttonID);
#else
        uint16_t tb = 0;
#endif
        //Record min/max
        if (tb < this->min_touch)
            this->min_touch = tb;
        else if (tb>this->max_touch)
            this->max_touch = tb;

        //Recalulate the middle threshold
        if (this->autocal)
        {
            this->touch_th = (this->min_touch + this->max_touch) / 2;
        }    


        if ((tb > this->touch_th) && (this->buttonPressed == false))
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

void ButtonClass::setTouchThresholds(uint16_t touch_th_low,uint16_t touch_th_high)
{
    this->min_touch = touch_th_low;
    this->max_touch = touch_th_high;
    this->touch_th =  (this->min_touch + this->max_touch) / 2;
}

void ButtonClass::setTouchMode(bool touch)
{
    this->isTouch = touch;
}

void ButtonClass::setAutocal(bool a)
{
    this->autocal = a;
}


uint16_t ButtonClass::getTouchThresholdHigh(void)
{
    return this->max_touch;
}

uint16_t ButtonClass::getTouchThresholdLow(void)
{
    return this->min_touch;
}

uint16_t ButtonClass::getTouchAnalog(uint16_t max)
{
#if BABOI_HW_VER == 2        
    uint16_t tb = touchRead(this->buttonID);
#else
    uint16_t tb = 0;
#endif
    //Record min/max
    if (tb < this->min_touch)
        this->min_touch = tb;
    else if (tb>this->max_touch)
        this->max_touch = tb;    

    int32_t val = (int32_t)map(tb,this->min_touch,this->max_touch,0,max); 
    return (uint16_t)val;
}