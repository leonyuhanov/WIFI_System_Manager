# WIFI_System_Manager
A web based configuration system for wearable applications using an ESP8266
An ESP32 version will come soon, there is a major diference in the way the webserver lib handles POST data

# A Note on using this in the real world
Im using the FLASH button on the ESP8266 module to put the device into config mode. In real world systems, you will need to change this to a usable GPIO pin

# Pre Flight
The main code has the following var
```C++
const char* defaultAnimationList = "vectorFadeSwirl,vectorTrace,pointTest,vectorSwirl,verticalMovingDrag,polyRotator,washMatrix,fallingRainDrops,linerGradientUp,xWave,rotationalGradientSwipe,linerGradientOut,midCircle,rainbowFader,outerCircle,verSlitRainbows,pondDrops,midRainDrops,fallingRainDrops,rainbowFaderUpStream,yWave,rainbowSwipe,midRainDrops,horizontalMovingDrag,smoothMatrix,colourDrag";
```
This C string is MY list of animation functions. For your application, you need to fill this out with whatever friendly names you like. EG if you have 5 animations with simple names like this:

```C++
const char* defaultAnimationList = "Animation1,Animation2,Animation3,Animation4,Animation5";
```
This list is used to populate the default configuration & is sent as the list of available animations to the UI. a "," is used a seperator. DO not use spaces or any charectors that your C compiler would like you to use in naming a function. Keep it simple

You will need then need to populate the file with your animations and create an ORDERED switch statment call for each
In the "data\index" file:
- You will need to populate the var "animationNames" in the same way
- Alter the var "animationDurations" with a list of detault durations per animation eg if you have 5 animations the list will be var animationDurations = "10,10,10,10,10"
```C++
void runAnimation(unsigned short int animationIndex, unsigned short int animationDuration)
{
  switch(animationIndex)
  {
    case  0:  startTimer(animationDuration);  
              animationOne(1);
              break;
              
    case  1:  startTimer(animationDuration);  
              animationTwo(1);
              break;
    case N:   .....
              break;
    default:    break;
  }
}
```
My animation systems use a set of functions that TIME their runtime, I have included them in this code:
```C++
startTimer(unsigned long durationInMillis);
//STart a timer to expire in 1 second
startTimer(1000);

while(!hasTimedOut())
{
  dome some animating
}
```
hasTimedOut() will return 1 if its timer has expired. This is how I do it, its not nessesarily the best way :)

# How to upload to your ESP8266

SET ESP SPIFS size to 2MB 

1. Upload to ESP8266 with "ERASE ALL FLASH CONTENTS"
2. Wait for SPIFS init(format takes 5-10 Seconds) on serail monitor. the esp will reboot after its created its default file system
3. Use FS manager to upload the file "index" in the data folder(ill remove the html its just there for ui testing)
4. Manualy restart the ESP via reboot push button

To load UI:
1. Connect to the ESP network

SSID: WOW-AP
KEY: wowaccesspoint
Web IP:  10.10.10.1

2. Open 10.10.10.1 in your devices browser
3. onLoad calls init()
4. init() calls configureAnimations() which builds the animation database
5. init() calls buildTable() to build the default UI with the default QUE (kind of redundant but will leave for now)
6. init() calls pull() which requests a BINARY data stream in GIF format (it works ok)
7. pull() stores the data in a byteArray the 1st half are the animation indexes, the second half are the durations
8. pull() clears UI and rebuilds the UI table

UI & ESP now support PUSH to save the UI config to the ESP SPIFS

