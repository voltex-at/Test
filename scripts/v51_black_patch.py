from pathlib import Path
import re

sketch = next(Path("sketch").glob("Weihnachtsuhr_v*_ES3C28P"))
main = sketch / "main.cpp"
touch = sketch / "Touch.h"
web = sketch / "WebPage.h"

# Black ES3C28P only: FT6336G reports absolute coordinates. No calibration layer.
touch.write_text(r'''#pragma once
#include <Arduino.h>
#include <Wire.h>

namespace touch {
constexpr int SDA_PIN=16;
constexpr int SCL_PIN=15;
constexpr int RST=18;
constexpr int IRQ=17;
constexpr uint8_t ADDR=0x38;

struct Point { int x=0, y=0; };

inline bool readBytes(uint8_t reg,uint8_t* data,size_t len){
  Wire.beginTransmission(ADDR);
  Wire.write(reg);
  if(Wire.endTransmission(false)!=0) return false;
  const size_t got=Wire.requestFrom((uint8_t)ADDR,(uint8_t)len);
  if(got!=len) return false;
  for(size_t i=0;i<len;++i) data[i]=Wire.read();
  return true;
}

inline void begin(){
  Wire.begin(SDA_PIN,SCL_PIN);
  Wire.setClock(400000);
  pinMode(RST,OUTPUT);
  pinMode(IRQ,INPUT);
  digitalWrite(RST,LOW);
  delay(15);
  digitalWrite(RST,HIGH);
  delay(320);
}

inline bool raw(Point& p){
  uint8_t count=0;
  if(!readBytes(0x02,&count,1) || (count&0x0F)==0) return false;

  uint8_t b[4]={};
  if(!readBytes(0x03,b,4)) return false;

  const uint16_t px=((b[0]&0x0F)<<8)|b[1];
  const uint16_t py=((b[2]&0x0F)<<8)|b[3];

  // FT6336G is mounted in portrait orientation while LCD works in landscape.
  const int x=int(py);
  const int y=239-int(px);
  if(x<0 || x>=320 || y<0 || y>=240) return false;
  p={x,y};
  return true;
}
}
''')

s = main.read_text()

# Strip all legacy calibration state.
s = re.sub(r'bool reconnect=false,calibrating=false,touchHeld=false,calibrationReady=false,bootConsumed=false;',
           'bool reconnect=false,touchHeld=false,bootConsumed=false;', s)
s = re.sub(r'int calibrationStep=0;\s*touch::Point calibrationRaw\[3\];\s*const touch::Point targets\[3\]=\{\{22,22\},\{298,22\},\{22,218\}\};\s*touch::Calibration calibration\{\};',
           '', s)

# Remove legacy calibration UI/functions completely if still present.
s = re.sub(r'void drawCalibration\(\) \{.*?\n\}\s*void startCalibration\(\)\{.*?\n\}', '', s, flags=re.S)
s = re.sub(r'void drawCalibration\(\)\{.*?\n\}\s*void startCalibration\(\)\{.*?\n\}', '', s, flags=re.S)

# Remove any old NVS load of touch calibration.
s = re.sub(r'\s*calibrationReady\s*=\s*[^;]+;','',s)
s = re.sub(r'\s*p\.getBytes\("touch"[^;]+;','',s)
s = re.sub(r'\s*Preferences\s+tp;\s*if\(tp\.begin\("peppi",true\)\)\{.*?tp\.end\(\);\s*\}','',s,flags=re.S)

# No touch calibration/test launch from old configuration screen.
s = s.replace('else startCalibration();','')
s = s.replace('startCalibration();','')

# Make black-board identity explicit in serial/build strings.
s = s.replace('Weihnachtsuhr v5.0.1 LVGL / ES3C28P',
              'Weihnachtsuhr v5.1 LVGL / BLACK ES3C28P')
s = s.replace('Weihnachtsuhr v5.0 LVGL / ES3C28P',
              'Weihnachtsuhr v5.1 LVGL / BLACK ES3C28P')

# Ensure no legacy XPT2046 vocabulary survives in main source.
for bad in ('XPT2046','TOUCH_X_MIN','TOUCH_X_MAX','TOUCH_Y_MIN','TOUCH_Y_MAX'):
    if bad in s:
        raise SystemExit('legacy touch token remains: '+bad)

main.write_text(s)

w = web.read_text()
for phrase in [
    'Touch-Kalibrierung','Touch Kalibrierung','Kalibrierung',
    'Kalibracja dotyku','Kalibracja','Touch calibration','Calibration'
]:
    w = w.replace(phrase,'Touch')
w = w.replace('v5.0.1 LVGL ES3C28P','v5.1 LVGL BLACK ES3C28P')
w = w.replace('v5.0 LVGL ES3C28P','v5.1 LVGL BLACK ES3C28P')
web.write_text(w)

# Final guard: firmware for this branch must contain only FT6336G touch code.
joined = main.read_text() + touch.read_text()
if 'Calibration' in joined or 'calibration' in joined:
    raise SystemExit('calibration code still present')
if 'FT6336G' not in joined:
    raise SystemExit('FT6336G driver missing')
