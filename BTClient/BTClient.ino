/*
  Copyright (c) 2014 MediaTek Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   See the GNU Lesser General Public License for more details.
*/
#include <LBT.h>
#include <LBTClient.h>
#include <LAudio.h>
#include <Adafruit_VC0706.h>
#include <LSD.h>
#include <LTask.h>
#include <LWiFi.h>
#include <LWiFiClient.h>

#define WIFI_AP "testEE"
#define WIFI_PASSWORD "your_password"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP according to your WiFi AP configuration


#define TEMP_BUF_SIZE (2048)
uint8_t buf[TEMP_BUF_SIZE] = {0};
IPAddress server(140,116,215,68);

char outBuf[128];
char outCount;

LWiFiClient c;
LWiFiClient dc;

char fileName[13] = "IMAGE00.jpg\r";
char file1[12] = "IMAGE00.jpg";
Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1);


static LBTDeviceInfo info = {0};
boolean find = 0;
#define SPP_SVR "master1" // it should be the prefer server's name,  customize it yourself.
#define ard_log Serial.printf
int read_size = 0;
const int motorIn1 = 4;
const int motorIn2 = 5;
const int motorIn3 = 6;     
const int motorIn4 = 7;  
LFile imgFile;

int i=0;
void setup()  
{
  
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  pinMode(motorIn3, OUTPUT);
  pinMode(motorIn4, OUTPUT);  
  Serial.begin(9600);
  LAudio.begin();
  Serial1.begin(38400);
  LWiFi.begin();
  LAudio.begin();
  LSD.begin();
  
  Serial.println("Connecting to AP");
  while (0 == LWiFi.connect(WIFI_AP,LWiFiLoginInfo(WIFI_AUTH, WIFI_PASSWORD)))
  {
    Serial.println("Connecting");
    delay(1000);
  }
  Serial.println("Connect OK!");
  
  ard_log("LBT start\n");
  // begin BT
  bool success = LBTClient.begin();
  if( !success )
  {
      ard_log("Cannot begin Bluetooth Client successfully\n");
      delay(0xffffffff);
  }
  else
  {
      ard_log("Bluetooth Client begin successfully\n");
      // scan the devices around
      int num = LBTClient.scan(30);
      ard_log("scanned device number [%d]", num);
      for (int i = 0; i < num; i++)
      {
        memset(&info, 0, sizeof(info));
        // to check the prefer master(server)'s name
        if (!LBTClient.getDeviceInfo(i, &info))
        {
            continue;
        }
        ard_log("getDeviceInfo [%02x:%02x:%02x:%02x:%02x:%02x][%s]", 
            info.address.nap[1], info.address.nap[0], info.address.uap, info.address.lap[2], info.address.lap[1], info.address.lap[0],
            info.name);
        if (0 == strcmp(info.name, SPP_SVR))
        {
            find = 1;
            ard_log("found\n");
            break;
        }
      }
      
  }
}
 
 
void snapshot()
{
  
  // Try to locate the camera
  if (cam.begin(38400)) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
   // return;
  } 
  cam.setImageSize(VC0706_640x480);
  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = cam.getImageSize();

  Serial.println("Snap in 3 secs...");
  delay(3000);

  if (! cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Create an image with the name IMAGExx.JPG
  char filename[13];
  strcpy(filename, "IMAGE00.jpg");
  if (! LSD.exists(filename)) LSD.remove(filename);
  
  // Open the file for writing
  imgFile = LSD.open(filename, FILE_WRITE);

  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");

  // Read all the data up to # bytes!
  byte wCount = 0; // For counting # of writes
  while (jpglen > 0) {
    // read 32 bytes at a time;
    uint8_t *buffer;
    uint8_t bytesToRead = min(32, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
    buffer = cam.readPicture(bytesToRead);
    imgFile.write(buffer, bytesToRead);
    if(++wCount >= 64) { // Every 2K, give a little feedback so it doesn't appear locked up
      Serial.print('.');
      wCount = 0;
    }
    //Serial.print("Read ");  Serial.print(bytesToRead, DEC); Serial.println(" bytes");
    jpglen -= bytesToRead;
  }
  imgFile.close();
  imgFile = LSD.open(filename);
  Serial.println("Connecting to FTP server");
  
  
  while (0 == c.connect(server, 2010))
  {
    Serial.println("Re-Connecting to FTP");
    delay(1000);
  }
  if(!eRcv()) return;

  c.println("USER embbeded\r");
  
  if(!eRcv()) return;

  c.println(F("PASS embbeded\r"));

    if(!eRcv()) return;

  c.println(F("SYST\r"));

  if(!eRcv()) return;

  c.println(F("PASV\r"));

  if(!eRcv()) return;

  char *tStr = strtok(outBuf,"(,");
  int array_pasv[6];
  for ( int i = 0; i < 6; i++) {
    tStr = strtok(NULL,"(,");
    array_pasv[i] = atoi(tStr);
    if(tStr == NULL)
    {
      Serial.println(F("Bad PASV Answer"));    

    }
  }

  unsigned int hiPort,loPort;

  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  Serial.print(F("Data port: "));
  hiPort = hiPort | loPort;
  Serial.println(hiPort);

  if (dc.connect(server,hiPort)) {
    Serial.println(F("Data connected"));
  }
  else {
    Serial.println(F("Data connection failed"));
    c.stop();
    imgFile.close();
    return;
  }


  c.print(F("STOR "));
  c.println(fileName);

  Serial.println(F("test"));
  if(!eRcv())
  {
    dc.stop();
    Serial.println(F("error"));
    return;
  }


  Serial.println(F("Writing"));

  byte clientBuf[64];
  int clientCount = 0;
  int imgSize=imgFile.size();

  while(dc.connected()&&imgSize>0)
  {
    
      clientBuf[clientCount] = imgFile.read();
    clientCount++;
    imgSize--;
    if(clientCount > 63)
    {
      dc.write(clientBuf,64);
      clientCount = 0;
    }
      
  }
if(clientCount > 0) dc.write(clientBuf,clientCount);

  dc.stop();
  Serial.println(F("Data disconnected"));

  if(!eRcv()) return;

  c.println(F("QUIT"));

  if(!eRcv()) return;

  c.stop();
  Serial.println(F("Command disconnected"));

  imgFile.close();
  Serial.println(F("SD closed"));
  Serial.println("done!");

}

byte eRcv()
{
  byte respCode;
  byte thisByte;

  while(!c.available()) delay(1);

  respCode = c.peek();

  outCount = 0;

  while(c.available())
  {  
    thisByte = c.read();    
    Serial.write(thisByte);

    if(outCount < 127)
    {
      outBuf[outCount] = thisByte;
      outCount++;      
      outBuf[outCount] = 0;
    }
  }
  
  if(respCode >= '4')
  {
    efail();    
    return 0;  
  }

  return 1;
}

void efail()
{
  byte thisByte = 0;
  Serial.println("error");
  c.println(F("QUIT"));

  while(!c.available()) delay(1);

  while(c.available())
  {  
    thisByte = c.read();    
    Serial.write(thisByte);
  }

  c.stop();
  Serial.println(F("Command disconnected"));
  imgFile.close();
  Serial.println(F("SD closed"));
}


void loop()
{
    
    // to check if the connection is ready
    
    if(find && !LBTClient.connected())
    {
       // ard_log("Diconnected, try to connect\n");
        // do connect
        bool conn_result = LBTClient.connect(info.address,"1234");
       // ard_log("connect result [%d]", conn_result);
 
        if( !conn_result )
        {
          //  ard_log("Cannot connect to SPP Server successfully\n");
            delay(0xffffffff);
        }
        else
        {
            ard_log("Connect to SPP Server successfully\n");
        }
        find = 0;
    }
    
    if(LBTClient.available()){
       
       byte cmd = LBTClient.read();
       Serial.println(cmd);
       switch(cmd){
         
         
          case 0x43:
              //ldr
                  if(i<10){
                    LBTClient.write((uint8_t*)"1", 1);
                    delay(1000);
                    i++;
                    break;
                  }
                  else LBTClient.write((uint8_t*)"0", 1);
                  delay(1000);
                  break;
          case 0x0B:
          
          // alarm on
                  LAudio.playFile( storageFlash,(char*)"sample.mp3");
                  LAudio.setVolume(3);
                  Serial.println("play music");
                  LBTClient.write((uint8_t*)"1", 1);
                  break;
                  
            case 0x0C:
          
          // alarm off
                  
                  LAudio.setVolume(0);
                  Serial.println("music stop");
                  LBTClient.write((uint8_t*)"1", 1);
                  break;
                  
                  
           case 0x40:
           
           // ahead, get the distance ahead
           
                   Serial.println("ahead ?");
                   delay(1000);
                   LBTClient.write((uint8_t*)"1", 1);
                   break;
           case 0x42:
           
           // left, get the distance left
           
                   Serial.println("left ?");
                   delay(1000);
                   LBTClient.write((uint8_t*)"0", 1);
                   break;
                   
            case 0x41:
            
            // right, get the distance right
           
                   Serial.println("right ?");
                   delay(1000);
                   LBTClient.write((uint8_t*)"1", 1);
                   break;
             case 0x05:
             
               // move forward
                   Serial.println("05");
                   forward();
                   delay(1000);
                   LBTClient.write((uint8_t*)"1", 1);
                   break;
              case 0x07:
               // turn left
                   Serial.println("07");
                   left();
                   delay(1000);
                   LBTClient.write((uint8_t*)"1", 1);
                   break;
               case 0x06:
               // turn right
                     Serial.println("06");
                    right();
                    delay(1000);
                    LBTClient.write((uint8_t*)"1", 1);
                    break;
                default:
                  break;
                   
         
       }
      
      
    }

}


void motorstop()
{
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, LOW);
}
	 
void forward()
{
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn3, HIGH);
  digitalWrite(motorIn4, LOW);
}
	 
void backward()
{
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, HIGH);
  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, HIGH);
}
	 
	// Let right motor keep running, but stop left motor
void right()
{
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, LOW);
}
	
// Let left motor keep running, but stop right motor
void left()
{
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn3, HIGH);
  digitalWrite(motorIn4, LOW);
}
