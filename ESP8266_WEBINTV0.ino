#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"
#include <string>

const char* configFilePath="/configFile";
unsigned short int configFileSize=0;
const char* indexPageFilePath="/index";

ESP8266WebServer server(80);
const char *ssid = "WOW-AP";
const char *password = "wowaccesspoint";
unsigned short int indexFileSize=0;
IPAddress local_ip = IPAddress(10,10,10,1);
IPAddress gateway = IPAddress(10,10,10,1);
IPAddress subnet = IPAddress(255,255,255,0);

//Default settings
const unsigned short int animationListSize=360;
const byte defultQueLength = 26;
const char* defaultAnimationList = "vectorFadeSwirl,vectorTrace,pointTest,vectorSwirl,verticalMovingDrag,polyRotator,washMatrix,fallingRainDrops,linerGradientUp,xWave,rotationalGradientSwipe,linerGradientOut,midCircle,rainbowFader,outerCircle,verSlitRainbows,pondDrops,midRainDrops,fallingRainDrops,rainbowFaderUpStream,yWave,rainbowSwipe,midRainDrops,horizontalMovingDrag,smoothMatrix,colourDrag";
char** animationArray;
const int16_t defaultAnimationTimeout = 15;
//Custom settings
unsigned short int queLength = 0;
int16_t** animationQue;

//Standard system variables when inserted into working animation lib
const byte configButton = 0;
byte systemMode = 0;
unsigned short int animationIndex=0;
unsigned long timeData[3];

void setup()
{  
  Serial.begin(115200);
  Serial.printf("\r\n\r\n\r\n");

  //Init File System
  initFS();
  //Load animation que from SPIFS OR set up a new one into SPIFS and then load
  setupAnimationQue();
  
  //Set up Input button for testing
  pinMode(configButton, INPUT);
  systemMode = digitalRead(configButton);
  if(systemMode==1)
  {
    //Beggin standard animation system with whatever animtaion que the system has in the SPI flash
    Serial.printf("\r\n\tBooting into normal opration mode using que loaded from SPIFS....");
    WiFi.mode(WIFI_OFF);
  }
  else if(systemMode==0)
  {
    //Begin Config mode
    Serial.printf("\r\n\tBooting into WEB UI configuration mode...");
    //configure Access Point
    WiFi.mode(WIFI_AP);
    WiFi.enableAP(true);
    delay(100);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ssid, password, 1, 0, 2);
    delay(100);
    Serial.printf("\r\n\tWIFI_AP MAC\t");
    Serial.print(WiFi.softAPmacAddress());
    Serial.printf("\r\n\tAP IP Address\t");
    Serial.print(WiFi.softAPIP());
    //init Web server
    server.on("/", handleRoot);
    server.on("/pull", handlePull);
    server.on("/push", HTTP_POST, handlePush);
    server.begin();
   }
  
   
}

void initFS()
{
  Serial.printf("\r\n\tSetting up SPIFS...");
  SPIFFS.begin();
  Serial.printf("\tSPIFS READY!");
}

void setupAnimationQue()
{
  unsigned short int qCnt=0, aLCnt=0, blockSize=0;
  unsigned short int textPos[2] = {0,0};
  byte dataIn[2] = {0,0};
  int16_t usiIn=0, usiIn2=0;
  File fileObject;
  
  //Build the default animation array list
  qCnt=0;
  animationArray = new char*[defultQueLength];
  queLength = defultQueLength;
  while(aLCnt<animationListSize)
  {
    //Start of current txt block
    textPos[0] = aLCnt;
    while(defaultAnimationList[aLCnt]!=',' && aLCnt+1<=animationListSize)
    {
      aLCnt++;
    }
    //End of current txt block
    textPos[1] = aLCnt;
    //Size of text block in array
    blockSize = textPos[1]-textPos[0];
    //init the char array with blockSize+1
    animationArray[qCnt] = new char[blockSize+1];
    //copy blockSize bytes into new char array
    memcpy(animationArray[qCnt], defaultAnimationList+textPos[0], blockSize);
    //terminate the char array with a NULL char
    animationArray[qCnt][blockSize]=0;
    qCnt++;
    aLCnt++;
  }
  
  //Read the configuration file
  fileObject = SPIFFS.open(configFilePath, "r");
  if(!fileObject)
  {
    //  IF NO CONFIG FILE EXISTS IN SPIFS CREATE ONE
    //  Reset to default
    //  Set up the animation que array
    animationQue = new int16_t*[queLength];
    for(qCnt=0; qCnt<queLength; qCnt++)
    {
      //init a new que item
      animationQue[qCnt] = new int16_t[2];
      //set the animation index to default ordered list index qCnt
      animationQue[qCnt][0] = qCnt;
      //set the animation timeout to the default defaultAnimationTimeout
      animationQue[qCnt][1] = defaultAnimationTimeout;
    }
    //create a config file and dump to SPIFS
    fileObject = SPIFFS.open(configFilePath, "w");
    if(!fileObject)
    {
      //somethign went wrong here
      Serial.printf("\r\n\r\nSOME KIND OF WEIRD EROROR??? CAN NOT OPEN FILE FOR WRITING");
    }
    else
    {
      for(qCnt=0; qCnt<queLength; qCnt++)
      {
        configFileSize += fileObject.write((byte*)&animationQue[qCnt][0], sizeof(animationQue[qCnt][0]));
        configFileSize += fileObject.write((byte*)&animationQue[qCnt][1], sizeof(animationQue[qCnt][1]));
      }
      fileObject.close();
      Serial.printf("\r\nDefault Configuration file saved to SPIFS\tWrote\t%d\tbytes Rebooting...", configFileSize);
      ESP.restart();
    }
  }
  else
  {
    //Read config data from config file
    configFileSize=fileObject.size();
    queLength = configFileSize/4;
    //Set up the animation que array
    animationQue = new int16_t*[queLength];
    Serial.printf("\r\nReading config file...\t%dbytes\t%dItems", configFileSize, queLength);
    for(qCnt=0; qCnt<queLength; qCnt++)
    {
      animationQue[qCnt] = new int16_t[2];
      
      //read animation index
      for(aLCnt=0; aLCnt<2; aLCnt++)
      {
        dataIn[aLCnt] = fileObject.read();
      }
      usiIn=0;
      usiIn2=0;
      usiIn = dataIn[1];
      usiIn = usiIn << 8;
      usiIn2 = dataIn[0];
      usiIn = usiIn | usiIn2;
      animationQue[qCnt][0] = usiIn;
      
      //read animation runtime
      for(aLCnt=0; aLCnt<2; aLCnt++)
      {
        dataIn[aLCnt] = fileObject.read();
      }
      usiIn=0;
      usiIn2=0;
      usiIn = dataIn[1];
      usiIn = usiIn << 8;
      usiIn2 = dataIn[0];
      usiIn = usiIn | usiIn2;
      animationQue[qCnt][1] = usiIn;
    }
    fileObject.close(); 
    Serial.printf("\r\nConfiguration loaded!"); 
  }
}

void loop()
{
  //Config Mode
  if(systemMode==0)
  {
    File fileObject; 
    indexFileSize = 0;
    fileObject = SPIFFS.open(indexPageFilePath, "r");
    if(!fileObject)
    {
        Serial.println("\r\n\tFAILED to open UI file for reading please UPLOAD index file from DATA directory via SKETCH DATA UPLAOD TOOL");
        delay(100000);
        return;
    }
    else
    {
      Serial.print("\r\n\tUI File present");
      indexFileSize = fileObject.size();
      fileObject.close();
      Serial.printf("\t%d\tBytes\r\n", indexFileSize); 
      blankFunction();
    }
  }
  //System Mode
  else if(systemMode==1)
  {
    Serial.printf("\r\nLaunch animation\t%d\tfor\t%d\t seconds\tQue Index\t%d", animationQue[animationIndex][0], animationQue[animationIndex][1], animationIndex);
    //Launch animation
    runAnimation(animationQue[animationIndex][0], animationQue[animationIndex][1]*1000);
    //Once animation times out increment animation index
    animationIndex = (animationIndex+1)%queLength;
  }
}

void runAnimation(unsigned short int animationIndex, unsigned short int animationDuration)
{
  switch(animationIndex)
  {
    case  0:  startTimer(animationDuration);  
              animationOne(1);
              break;
              
    case  1:  startTimer(animationDuration);  
              animationOne(1);
              break;
              
    default:    break;
  }
}

//animations
void animationOne(byte colourIncrement)
{
  //A blank animation template
  while(true)
  {
    if(hasTimedOut())
    {
      return;
    }
    yield();
  }
}
//animations

void blankFunction()
{
  Serial.printf("\r\nSystem Ready\r\n");
  while(true)
  {
    server.handleClient();
    yield();
  }
}
void handleRoot()
{
  File tempFile;
  char* indexFile;
  
  if(indexFileSize>0)
  {
    indexFile = new char[indexFileSize];
    //read in file
    tempFile = SPIFFS.open(indexPageFilePath, "r");
    tempFile.readBytes(indexFile, indexFileSize);
    tempFile.close();
    server.send(200, "text/html", indexFile);
  }
  Serial.printf("\r\nServed UI \tSent\t%d Bytes", indexFileSize);
}

void handlePull()
{
  char animationIndexList[queLength*2];
  unsigned short int qCnt=0, innerQcnt=0;
  
  //FIll animation indexs
  for(qCnt=0; qCnt<queLength; qCnt++)
  {
    animationIndexList[qCnt] = animationQue[qCnt][0];
  }  
  //FILL animation durations
  for(qCnt; qCnt<queLength*2; qCnt++)
  {
    animationIndexList[qCnt] = animationQue[innerQcnt][1];
    innerQcnt++;
  }  
  server.sendHeader("Access-Control-Allow-Origin", "*", true);
  server.send(200, "image/gif", animationIndexList, queLength*2);
  Serial.printf("\r\nServed PULL request\tSent\t%d Bytes", queLength*2);
}
void handlePush()
{
  unsigned short int dataCnt=0, qCnt=0, configFileSize=0;
  File fileObject;
  
  //Respond to the UI
  server.sendHeader("Access-Control-Allow-Origin", "*", true);
  server.send(200, "text/html", "OK");

  //Recreate the animation que array
  queLength = server.arg(0).length()/2;
  animationQue = new int16_t*[queLength];
  for(qCnt=0; qCnt<queLength; qCnt++)
  {
    //init a new que item
    animationQue[qCnt] = new int16_t[2];
    //set the animation index to default ordered list index qCnt
    animationQue[qCnt][0] = server.arg(0)[qCnt];
    //set the animation timeout to the default defaultAnimationTimeout
    animationQue[qCnt][1] = server.arg(0)[queLength+qCnt];
  }
    
  //create new config file and reboot
  fileObject = SPIFFS.open(configFilePath, "w");
  for(qCnt=0; qCnt<queLength; qCnt++)
  {
    configFileSize += fileObject.write((byte*)&animationQue[qCnt][0], sizeof(animationQue[qCnt][0]));
    configFileSize += fileObject.write((byte*)&animationQue[qCnt][1], sizeof(animationQue[qCnt][1]));
  }
  fileObject.close();
  Serial.printf("\r\Custom Configuration file saved to SPIFS\t Wrote\t%d\tbytes\r\nRebooting...", configFileSize);
  ESP.restart();
}
byte hasTimedOut()
{
  timeData[1] = millis();
  if(timeData[2] < timeData[1]-timeData[0])
  {
    return 1;
  }
  return 0;
}
void startTimer(unsigned long durationInMillis)
{
  timeData[0] = millis(); 
  timeData[2] = durationInMillis;
}
