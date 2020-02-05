#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"

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
const int16_t defaultAnimationTimeout = 60;
//Custom settings
unsigned short int queLength = 0;
int16_t** animationQue;

void setup()
{  
  Serial.begin(115200);
  Serial.printf("\r\n\r\n\r\n");

  //configure Access Point
  WiFi.mode(WIFI_AP);
  WiFi.enableAP(true);
  delay(100);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  WiFi.softAP(ssid, password, 1, 0, 2);
  delay(100);
  
  Serial.printf("\r\n\tWIFI_AP MAC\t");
  Serial.println(WiFi.softAPmacAddress());
  Serial.printf("AP IP Address\t");
  Serial.print(WiFi.softAPIP());

  //init Web server
  server.on("/", handleRoot);
  server.on("/pull", handlePull);
  server.on("/push", handlePush);
  
  server.begin();

  //init FS
  SPIFFS.begin();

  setupAnimationQue();
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
  /*
  //verify list
  Serial.printf("\r\nImported\t%d\tanimations", qCnt);
  for(qCnt=0; qCnt<defultQueLength; qCnt++)
  {
    Serial.printf("\r\n%d", qCnt);
    blockSize = strlen(animationArray[qCnt]);
    Serial.printf("\t%dbytes\t[", blockSize);
    Serial.print(animationArray[qCnt]);
    Serial.printf("]");
  }
  */
  
  //Read the configuration file
  fileObject = SPIFFS.open(configFilePath, "r");
  if(!fileObject)
  {
    //Reset to default
    //Set up the animation que array
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
    }
    else
    {
      for(qCnt=0; qCnt<queLength; qCnt++)
      {
        configFileSize += fileObject.write((byte*)&animationQue[qCnt][0], sizeof(animationQue[qCnt][0]));
        configFileSize += fileObject.write((byte*)&animationQue[qCnt][1], sizeof(animationQue[qCnt][1]));
      }
      Serial.printf("\r\nWrote\t%d\tbytes", configFileSize);
      //delete file for testing
      fileObject.close();
      //SPIFFS.remove(configFilePath);
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
    
    //delete for testing
    fileObject.close();
    SPIFFS.remove(configFilePath);    
    /*
    //verify load
    for(qCnt=0; qCnt<queLength; qCnt++)
    {
      Serial.printf("\r\n%d\t[", qCnt);
      Serial.print( animationArray[animationQue[qCnt][0]] );
      Serial.printf("]\t[%d]", animationQue[qCnt][1]);
    }
    */
  }
}

void loop()
{
  File fileObject;
  
  indexFileSize = 0;
  fileObject = SPIFFS.open(indexPageFilePath, "r");
  if(!fileObject)
  {
      Serial.println("\r\n- failed to open file for reading");
      delay(100000);
      return;
  }
  else
  {
    Serial.print("\r\nUI File present");
    indexFileSize = fileObject.size();
    fileObject.close();
    Serial.printf("\t%d\tBytes\r\n", indexFileSize); 
    blankFunction();
  }
}
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
  server.send(200, "image/gif", animationIndexList, queLength*2);
  Serial.printf("\r\nServed PULL request\tSent\t%d Bytes", queLength*2);
}
void handlePush()
{

}
