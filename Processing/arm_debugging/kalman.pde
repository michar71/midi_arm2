import java.util.*; 
import java.math.*;


class kalman {
    /* Kalman filter variables */
    protected float q; //process noise covariance
    protected float r; //measurement noise covariance
    protected float x; //value
    protected float p; //estimation error covariance
    protected float k; //kalman gain
    

    kalman(float process_noise, float sensor_noise, float estimated_error, float intial_value) 
    {
      /* The variables are x for the filtered value, q for the process noise, 
         r for the sensor noise, p for the estimated error and k for the Kalman Gain. 
         The state of the filter is defined by the values of these variables.
         
         The initial values for p is not very important since it is adjusted
         during the process. It must be just high enough to narrow down.
         The initial value for the readout is also not very important, since
         it is updated during the process.
         But tweaking the values for the process noise and sensor noise
         is essential to get clear readouts.
         
         For large noise reduction, you can try to start from: (see http://interactive-matter.eu/blog/2009/12/18/filtering-sensor-data-with-a-kalman-filter/ )
         q = 0.125
         r = 32
         p = 1023 //"large enough to narrow down"
         e.g.
         myVar = Kalman(0.125,32,1023,0);
      */
        this.q = process_noise;
        this.r = sensor_noise;
        this.p = estimated_error;
        this.x = intial_value; //x will hold the iterated filtered value
    }
    
    public float getFilteredValue(float measurement) 
    {
    /* Updates and gets the current measurement value */
    //prediction update
    //omit x = x
    this.p = this.p + this.q;
      
    //measurement update
    this.k = this.p / (this.p + this.r);
    this.x = this.x + this.k * (measurement - this.x);
    this.p = (1 - this.k) * this.p;
    
    return this.x;
    }
    
    public void setParameters(float process_noise, float sensor_noise, float estimated_error) 
    {
        this.q = process_noise;
        this.r = sensor_noise;
        this.p = estimated_error;
    }
    
    public float setValue( float intial_value)
    {
      float fval = 0;
      this.x = intial_value;
      this.p = 0.01;
      for (int ii=0;ii<16;ii++)
        fval = getFilteredValue(intial_value); 
      return fval;
    }    

    public void setParameters(float process_noise, float sensor_noise) 
    {
        this.q = process_noise;
        this.r = sensor_noise;
    }
    
    public float getProcessNoise() 
    {
      return this.q;
    }
    
    public float getSensorNoise() 
    {
      return this.r;
    }
    
    public float getEstimatedError() 
    {
      return this.p;
    }
    
    public float getKalmanGain()
    {
      return this.x;
    }
}
