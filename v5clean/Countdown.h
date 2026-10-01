#pragma once
#include "Calendar.h"
#include <stdint.h>
#include <string.h>
namespace countdown {
enum Mode:uint8_t {Christmas,Birthday,Valentines,Easter,Vacation,Halloween};
struct Config {
 uint32_t magic=0x43445431;
 uint8_t mode=Christmas,birthMonth=0,birthDay=0,vacMonth=0,vacDay=0,reserved=0;
 uint16_t vacYear=0;
 char name[49]={};
};
inline const char* code(uint8_t mode){const char* names[]={"christmas","birthday","valentines","easter","vacation","halloween"};return mode<=Halloween?names[mode]:names[0];}
inline int index(const char* s){for(int i=0;i<=Halloween;++i)if(!strcmp(s,code(i)))return i;return -1;}
inline bool validConfig(const Config& c){
 bool b=(!c.birthMonth&&!c.birthDay)||peppi::valid({2024,c.birthMonth,c.birthDay});
 bool v=(!c.vacYear&&!c.vacMonth&&!c.vacDay)||peppi::valid({c.vacYear,c.vacMonth,c.vacDay});
 return c.magic==0x43445431&&c.mode<=Halloween&&memchr(c.name,0,sizeof(c.name))&&b&&v;
}
inline bool configured(const Config& c){return validConfig(c)&&(c.mode!=Birthday||c.birthMonth)&&(c.mode!=Vacation||c.vacYear);}
// Gregorian Easter, Oudin algorithm documented by the US Naval Observatory.
inline peppi::Date easter(int y){int c=y/100,n=y%19,k=(c-17)/25,i=c-c/4-(c-k)/3+19*n+15;i%=30;
 i-=i/28*(1-i/28*(29/(i+1))*((21-n)/11));int j=(y+y/4+i+2-c+c/4)%7,l=i-j,m=3+(l+40)/44;
 return {y,m,l+28-31*(m/4)};}
struct Result {bool configured=false,today=false,started=false;int days=-1;peppi::Date target{0,0,0};};
inline Result calculate(peppi::Date now,const Config& c){
 Result r;r.configured=countdown::configured(c);if(!r.configured||!peppi::valid(now))return r;
 int y=now.year;
 switch(c.mode){
 case Christmas:r.target={y+(now.month==12&&now.day>26),12,24};break;
 case Valentines:r.target={y,2,14};break;
 case Halloween:r.target={y,10,31};break;
 case Easter:r.target=easter(y);break;
 case Vacation:r.target={c.vacYear,c.vacMonth,c.vacDay};break;
 case Birthday:
   while(c.birthDay>peppi::monthDays(y,c.birthMonth))++y;
   r.target={y,c.birthMonth,c.birthDay};break;
 }
 int delta=peppi::ordinal(r.target)-peppi::ordinal(now);
 if(c.mode==Christmas&&peppi::scene(now)==peppi::Scene::Christmas)delta=0;
 else if(delta<0&&c.mode!=Vacation){
   y=r.target.year+1;
   if(c.mode==Easter)r.target=easter(y);
   else {while(c.mode==Birthday&&c.birthDay>peppi::monthDays(y,c.birthMonth))++y;r.target.year=y;}
   delta=peppi::ordinal(r.target)-peppi::ordinal(now);
 }
 r.today=delta==0;r.started=c.mode==Vacation&&delta<=0;r.days=delta<0?0:delta;return r;
}
}
