# midi_arm2

## BABOI Update
1) Download .bin Firmware File
1) Power Up BABOI
2) Connect Computer o Phone to BABOI Access Point
3) Go to webapge 192.168.1.1 in browser
4) Select "Update"
5) Select firmware File

##Processing Sketch update
Just copy the .app file in the relase folder to your computer 

## LED's

**Left LED**

Red,Green,Blue -> Boot
Yellow -> Ready for Connection
Green -> Connection initiated

**Right LED**

Off > Not Connected
Red > Pause
Green > Live
Blue > Acc/Gyro Cal
Cyan > Mag Cal
Magenta > Button Cal
White > Glove Tension Strip Cal

## Buttons

**Left**

CTRL-Button
Short Press > Live/Pause Toggle
Long Press > Gyro/Accelerometer Calibration
Very Long Press > Mag Calibration

**Front**

A-Button

**Right**

B-Button

## Calibration
- Best Way to calibrate is from the webpage
- Buttons might not work correctly initially. Calibrate from Webpage...
- Gyro/Accelerometer Calibration can be triggered both by 3 sec button press or webpage.
- Button/Mag calibration can only be triggered from webpage.
- If in doubt reset settings from webpage... (That will force a button recalibration on next start...)

## Network
- Enable Network support in App
- Connect computer to "BABOI" access point
- Web Page is available at 192.168.1.1
- VPN/Firewall might enterfere. Traffic is on Port 2255. Connection is established via Broadcast, then communication switches to unicast. 
 
