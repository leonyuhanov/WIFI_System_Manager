#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "FS.h"

const char* configFilePath="/configFile";
const char* indexPageFilePath="/index";
ESP8266WebServer server(80);
const char *ssid = "WOW-AP";
const char *password = "wowaccesspoint";
char* indexFile;
unsigned short int indexFileSize=0;
IPAddress local_ip = IPAddress(10,10,10,1);
IPAddress gateway = IPAddress(10,10,10,1);
IPAddress subnet = IPAddress(255,255,255,0);

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
  server.begin();

  //init FS
  SPIFFS.begin();
}

void loop()
{
  indexFileSize = 0;
  File file = SPIFFS.open(indexPageFilePath, "r");
  if(!file)
  {
      Serial.println("\r\n- failed to open file for reading");
      delay(100000);
      return;
  }
  else
  {
    Serial.print("\r\nRead from file: \r\n");
    while(file.available())
    {
      Serial.write(file.read());
      indexFileSize++;
    }
    file.close();
    Serial.printf("\r\nRead\t%d\tBytes.", indexFileSize); 
    indexFile = new char[indexFileSize];

    //read in file
    file = SPIFFS.open(indexPageFilePath, "r");
    file.readBytes(indexFile, indexFileSize);
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
  if(indexFileSize>0)
  {
    server.send(200, "text/html", indexFile);
  }
  
}
