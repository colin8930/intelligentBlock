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

static LBTDeviceInfo info = {0};
boolean find = 0;
#define SPP_SVR "master1" // it should be the prefer server's name,  customize it yourself.
#define ard_log Serial.printf
int read_size = 0;
const int motorIn1 = 4;
const int motorIn2 = 5;
const int motorIn3 = 6;     
const int motorIn4 = 7;  


int i=0;
void setup()  
{
  
  pinMode(motorIn1, OUTPUT);
  pinMode(motorIn2, OUTPUT);
  pinMode(motorIn3, OUTPUT);
  pinMode(motorIn4, OUTPUT);  
  Serial.begin(9600);
  LAudio.begin();
 
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
