#pragma once
#include <Arduino.h>
#include <Wire.h>

// BLACK ES3C28P: capacitive FT6336G, absolute coordinates, no calibration.
namespace touch {
constexpr int SDA_PIN=16,SCL_PIN=15,RST=18,IRQ=17;
constexpr uint8_t ADDR=0x38;
struct Point {int x=0,y=0;};
inline bool readBytes(uint8_t reg,uint8_t* data,size_t len){
  Wire.beginTransmission(ADDR);Wire.write(reg);if(Wire.endTransmission(false)!=0)return false;
  size_t got=Wire.requestFrom((uint8_t)ADDR,(uint8_t)len);if(got!=len)return false;
  for(size_t i=0;i<len;++i)data[i]=Wire.read();return true;
}
inline void begin(){
  Wire.begin(SDA_PIN,SCL_PIN);Wire.setClock(400000);pinMode(RST,OUTPUT);pinMode(IRQ,INPUT);
  digitalWrite(RST,LOW);delay(15);digitalWrite(RST,HIGH);delay(320);
}
inline bool raw(Point& p){
  uint8_t count=0;if(!readBytes(0x02,&count,1)||(count&0x0F)==0)return false;
  uint8_t b[4]={};if(!readBytes(0x03,b,4))return false;
  const uint16_t px=((b[0]&0x0F)<<8)|b[1],py=((b[2]&0x0F)<<8)|b[3];
  const int x=(int)py,y=239-(int)px;
  if(x<0||x>=320||y<0||y>=240)return false;p={x,y};return true;
}
}
