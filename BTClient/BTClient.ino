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
#include <LFlash.h>
#include <LWiFi.h>
#include <LWiFiClient.h>
#include <LTask.h>
#include <LGSM.h>

#define WIFI_AP "testEE"
#define WIFI_PASSWORD "your_password"
#define WIFI_AUTH LWIFI_WPA  // choose from LWIFI_OPEN, LWIFI_WPA, or LWIFI_WEP according to your WiFi AP configuration
#define TRIGPINA  6
#define ECHOPINA  7
#define TRIGPINB  8
#define ECHOPINB  9
#define TRIGPINC  2
#define ECHOPINC  3
#define PWMHIGH 200
#define PWMLOW 0
#define TEMP_BUF_SIZE (2048)
#define LEDA 12
#define LEDB 13

uint8_t buf[TEMP_BUF_SIZE] = {0};
IPAddress server(140,116,215,68);

char outBuf[128];
char outCount;

LWiFiClient c;
LWiFiClient dc;

char fileName[13] = "IMAGE00.jpg\r";
char file1[12] = "IMAGE00.jpg";
Adafruit_VC0706 cam = Adafruit_VC0706(&Serial1);
char phoneNumber[11] = "0933343268"; 

static LBTDeviceInfo info = {0};
boolean find = 0;
#define SPP_SVR "master1" // it should be the prefer server's name,  customize it yourself.
#define ard_log Serial.printf
int read_size = 0;
const int motorIn1 = 10;
const int motorIn2 = 11;
const int motorIn3 = 5;     
const int motorIn4 = 4;  
LFile imgFile;
char toSend;
int i=0;
void setup()  
{
  LTask.begin();
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  pinMode(motorIn3, OUTPUT);
  pinMode(motorIn4, OUTPUT);  
  pinMode(TRIGPINA, OUTPUT);
  pinMode(ECHOPINA, INPUT);
  pinMode(TRIGPINB, OUTPUT);
  pinMode(ECHOPINB, INPUT);
  pinMode(TRIGPINC, OUTPUT);
  pinMode(ECHOPINC, INPUT);
  pinMode(LEDA, OUTPUT);
  pinMode(LEDB, OUTPUT);
  digitalWrite(motorIn1,LOW);
  digitalWrite(motorIn2,LOW);
  digitalWrite(motorIn3,LOW);
  digitalWrite(motorIn4,LOW);
  digitalWrite(LEDA,HIGH);
  digitalWrite(LEDB,HIGH);
  Serial.begin(9600);
  LAudio.begin();
  Serial1.begin(38400);
  LWiFi.begin();
  LAudio.begin();
  LFlash.begin();
  
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
      int num = LBTClient.scan(15);
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
  
    Serial.println("Connecting to FTP server");
  
  
  while (0 == c.connect(server, 2010))
  {
    Serial.println("Re-Connecting to FTP");
    delay(1000);
  }
  if(!eRcv()) {
    Serial.println("error");
    return;
  }
  c.println("USER embbeded\r");
  
  if(!eRcv()) return;

  c.println(F("PASS embbeded\r"));

    if(!eRcv()) return;

  c.println(F("SYST\r"));

  if(!eRcv()) return;


}
 
 
void snapshot()
{
  Serial.println("test");
  // Try to locate the camera
  if (cam.begin(38400)) {
    Serial.println("Camera Found:");
  } else {
    Serial.println("No camera found?");
   // return;
  }
  cam.setImageSize(VC0706_320x240);
  
   uint8_t imgsize = cam.getImageSize();
  Serial.print("Image size: ");
  if (imgsize == VC0706_640x480) Serial.println("640x480");
  if (imgsize == VC0706_320x240) Serial.println("320x240");
  if (imgsize == VC0706_160x120) Serial.println("160x120");

  Serial.println("Snap in 3 secs...");
  delay(1000);

  if (!cam.takePicture()) 
    Serial.println("Failed to snap!");
  else 
    Serial.println("Picture taken!");
  
  // Create an image with the name IMAGExx.JPG

  if (LFlash.exists(file1)) LFlash.remove(file1);
  imgFile = LFlash.open(file1, FILE_WRITE);


  // Open the file for writing
 // imgFile = LSD.open(file1, FILE_WRITE);
 //imgFile = LSD.open(filename, FILE_WRITE);
  // Get the size of the image (frame) taken  
  uint16_t jpglen = cam.frameLength();
  Serial.print("Storing ");
  Serial.print(jpglen, DEC);
  Serial.print(" byte image.");
  pinMode(8, OUTPUT);
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
  Serial.println("test2");
  delay(10);
  imgFile = LFlash.open(file1);

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
  
  Serial.print("1");
  c.println(F("STOR IMAGE00.jpg\r"));
 // c.print(F(filename));
 // c.print(F("\r\n"));
  
  Serial.print("2");
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
    Serial.println(imgSize);
  while(imgSize>0)
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
  if(!eRcv()) return;
  Serial.println(F("Data disconnected"));

  //if(!eRcv()) return;

 // c.println(F("QUIT\r"));

  //if(!eRcv()) return;

 // c.stop();
  //Serial.println(F("Command disconnected"));

  imgFile.close();
  Serial.println(F("SD closed"));
  Serial.println("done!");
  /*
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
  }
*/

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
  c.println(F("QUIT\r"));

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

long pingA() {
digitalWrite(TRIGPINA, LOW);
delayMicroseconds(2);
digitalWrite(TRIGPINA, HIGH);
delayMicroseconds(100);
digitalWrite(TRIGPINA, LOW);
return pulseIn(ECHOPINA, HIGH)/58;
} 


long pingB() {
digitalWrite(TRIGPINB, LOW);
delayMicroseconds(2);
digitalWrite(TRIGPINB, HIGH);
delayMicroseconds(100);
digitalWrite(TRIGPINB, LOW);
return pulseIn(ECHOPINB, HIGH)/58;
} 

long pingC() {
digitalWrite(TRIGPINC, LOW);
delayMicroseconds(2);
digitalWrite(TRIGPINC, HIGH);
delayMicroseconds(100);
digitalWrite(TRIGPINC, LOW);
return pulseIn(ECHOPINC, HIGH)/58;
} 

boolean ifAhead()
{
  long cm = pingA();
  Serial.println(cm);
  if(cm<20) return 1;
  else return 0;
  
}

boolean errDet()
{
  long cm = pingA();
  Serial.println(cm);
  if(cm<15) return 1;
  else return 0;
  
}

boolean ifLeft()
{
  long cm = pingB();
  Serial.println(cm);
  if(cm<20) return 1;
  else return 0;  
}

boolean ifRight()
{
  long cm = pingC();
  Serial.println(cm);
  if(cm<20) return 1;
  else return 0;
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
          //  ard_log("Cannot connect to SPP Server successfully\n");9
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
       int ran=analogRead(A0)%3;
       Serial.print("ran:");
       Serial.println(ran);
      
       
       switch(cmd){
            case 0x85:
                    LBTClient.write((uint8_t*)"1", 1);
                     for(int j=0; j<8;j++){
                       while(!LBTClient.available());
       
                         byte tmp = LBTClient.read();
                         Serial.println(tmp);
                         phoneNumber[j+2]=tmp;
                         if(j<7)    LBTClient.write((uint8_t*)"1", 1);
                     
                     }
                     Serial.println(phoneNumber);
                     Serial.println("test");                
                     SMS();
                     LBTClient.write((uint8_t*)"1", 1);
                      break;
            
            case 0x00:
                      LAudio.playFile( storageFlash,(char*)"surprise.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
          case 0x44:
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
                  
          #if 0
          case 0x0B:
          
          // alarm on
               //   LAudio.playFile( storageFlash,(char*)"sample.mp3");
               //   LAudio.setVolume(3);
                  Serial.println("play music");
                  LBTClient.write((uint8_t*)"1", 1);
                  break;
                  
            case 0x0C:
          
          // alarm off
                  
                  LAudio.setVolume(0);
                  Serial.println("music stop");
                  LBTClient.write((uint8_t*)"1", 1);
                  break;
            #endif    
                  
           case 0x40:
           
           // ahead, get the distance ahead
           
                   Serial.println("ahead ?");
                   delay(100);
                   if(ifAhead())  LBTClient.write((uint8_t*)"1", 1);
                   else LBTClient.write((uint8_t*)"0", 1);
                   break;
           case 0x42:
           
           // left, get the distance left
           
                   Serial.println("left ?");
                   delay(100);
                   if(ifLeft())  LBTClient.write((uint8_t*)"1", 1);
                   else LBTClient.write((uint8_t*)"0", 1);
                   break;
                   
            case 0x41:
            
            // right, get the distance right
           
                   Serial.println("right ?");
                   delay(100);
                   if(ifRight())  LBTClient.write((uint8_t*)"1", 1);
                   else LBTClient.write((uint8_t*)"0", 1);
                   break;
             case 0x05:
             
               // move forward
                   Serial.println("05");
                   //error detection
                   if(errDet()) {
                       LAudio.playFile( storageFlash,(char*)"surprise.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                   }
                   
                   forward();
                   delay(300);
                   motorstop();
                   LBTClient.write((uint8_t*)"1", 1);
                   break;
              case 0x07:
               // turn left
                   Serial.println("07");
                   left();
                   delay(300);
                   motorstop();
                   LBTClient.write((uint8_t*)"1", 1);
                   break;
               case 0x06:
               // turn right
                     Serial.println("06");
                    right();
                    delay(300);
                    motorstop();
                    LBTClient.write((uint8_t*)"1", 1);
                    break;
                 case 0x08:
                //Snapshot
                    Serial.println("08");
                    /*LAudio.playFile( storageFlash,(char*)"camera.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();*/
                    snapshot();
                    delay(1000);
                    LBTClient.write((uint8_t*)"1", 1);
                    break;
                 case 0x09:
                     Serial.println("09");
                     Calling();
                     delay(1000);
                     LBTClient.write((uint8_t*)"1", 1);
                     break;
                  case 0x0A:
                      LAudio.playFile( storageFlash,(char*)"cat.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                  case 0x0B:
                      LAudio.playFile( storageFlash,(char*)"coin.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                  case 0x0C:
                      LAudio.playFile( storageFlash,(char*)"crow.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                  case 0x0D:
                      LAudio.playFile( storageFlash,(char*)"kiss.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                  case 0x0E:
                      LAudio.playFile( storageFlash,(char*)"owl.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                  case 0x10:
                      LAudio.playFile( storageFlash,(char*)"piano.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                  case 0x11:
                      LAudio.playFile( storageFlash,(char*)"burp.mp3");
                      LAudio.setVolume(6);
                      delay(1000);
                      LAudio.stop();
                      LBTClient.write((uint8_t*)"1", 1);
                       break;
                       
                   case 0x12:
                       SMS();
                       delay(1000);
                       LBTClient.write((uint8_t*)"1", 1);
                       break;
                    case 0x13:
                        Calling();
                        delay(1000);
                        LBTClient.write((uint8_t*)"1", 1);
                        break;
                        
                    case 0x14:
                        ledON();
                        delay(1000);
                        break;
                    case 0x15:
                        ledOFF();
                        delay(1000);
                        break;
                    case 0x16:
                        ledBlink();
                        delay(1000);
                        break;
                     
                     
                    case 0x43:
                        //random sense
                        Serial.println("test");
                        switch(ran){
                          
                          case 0:
                          Serial.println(0);
                               LAudio.setVolume(6);
                               LAudio.playFile( storageFlash,(char*)"A.mp3");
                               LAudio.setVolume(6);
                               delay(800);
                               LAudio.stop();                              
                              if(ifAhead())  {
                                LAudio.setVolume(6);
                                LAudio.playFile( storageFlash,(char*)"coin.mp3");
                               
                               delay(1000);
                               LAudio.stop();
                                LBTClient.write((uint8_t*)"1", 1);
                              }
                              else{
                               // delay(1000);
                                LBTClient.write((uint8_t*)"0", 1);
                              }
                              break;
                           case 1:
                               Serial.println(1);
                               LAudio.setVolume(6);
                               LAudio.playFile( storageFlash,(char*)"B.mp3");
                               LAudio.setVolume(6);
                               delay(800);
                               LAudio.stop();                             
                              if(ifLeft())  {
                                LAudio.playFile( storageFlash,(char*)"coin.mp3");
                               LAudio.setVolume(6);
                               delay(1000);
                               LAudio.stop();
                                LBTClient.write((uint8_t*)"1", 1);
                              }
                              else{
                              // delay(1000);
                               LBTClient.write((uint8_t*)"0", 1);
                              }
                              break;
                            case 2:
                              Serial.println(2);
                              LAudio.setVolume(6);
                              LAudio.playFile( storageFlash,(char*)"C.mp3");
                               LAudio.setVolume(6);
                               delay(800);
                               LAudio.stop();      
                              
                              if(ifRight()) {
                                LAudio.playFile( storageFlash,(char*)"coin.mp3");
                               LAudio.setVolume(6);
                               delay(1000);
                               LAudio.stop();
                                LBTClient.write((uint8_t*)"1", 1);
                              }
                              else{
                                // delay(1000);
                                LBTClient.write((uint8_t*)"0", 1);
                              }break;
                        }
                default:
                  break;
                   
         
       }
      
      
    }

}

void ledON(){
  Serial.println("on");
  digitalWrite(LEDA, HIGH);
  digitalWrite(LEDB, HIGH);
}

void ledOFF(){
  Serial.println("off");
  digitalWrite(LEDA, LOW);
  digitalWrite(LEDB, LOW);
}

void ledBlink(){
  digitalWrite(LEDA, LOW);
  digitalWrite(LEDB, HIGH);
  delay(1000);
  digitalWrite(LEDB, LOW);
  digitalWrite(LEDA, HIGH);
  delay(1000);
  digitalWrite(LEDA, LOW);
  digitalWrite(LEDB, HIGH);
  delay(1000);
  digitalWrite(LEDA, LOW);
  digitalWrite(LEDB, LOW);
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
  digitalWrite(motorIn1, LOW);
  digitalWrite(motorIn2, HIGH);
  digitalWrite(motorIn3, HIGH);
  digitalWrite(motorIn4, LOW);
}
	
// Let left motor keep running, but stop right motor
void left()
{  
  digitalWrite(motorIn1, HIGH);
  digitalWrite(motorIn2, LOW);
  digitalWrite(motorIn3, LOW);
  digitalWrite(motorIn4, HIGH);
  
}

void Calling()
{ 
        Serial.print("Calling \n");   
        
        // Check if the receiving end has picked up the call
        if(LVoiceCall.voiceCall(phoneNumber))
        {
          Serial.println("Call Established. Enter line to end");
          // Wait for some input from the line
          while(Serial.read() !='\n');
          // And hang up
          LVoiceCall.hangCall();
        }
        Serial.println("Call Finished");
}

void SMS()
{
      while(!LSMS.ready()){
        Serial.println("not ready");
       delay(1000);
      }
      Serial.println("SIM ready for work");
     LSMS.beginSMS(phoneNumber);
     LSMS.println("welcome to user final presentation");
     LSMS.print("password is: 123");
     
     if(LSMS.endSMS())
     {
         Serial.println("SMS is sent");
     }
     else
     {
          Serial.println("SMS is not sent");
     }
}
