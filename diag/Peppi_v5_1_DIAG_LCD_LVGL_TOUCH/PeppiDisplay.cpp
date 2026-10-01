#include "PeppiDisplay.h"
#include <math.h>
PeppiDisplay::PeppiDisplay():Adafruit_GFX(WIDTH,HEIGHT){}
void PeppiDisplay::command(uint8_t cmd){digitalWrite(PIN_DC,LOW);digitalWrite(PIN_CS,LOW);spi_.transfer(cmd);digitalWrite(PIN_CS,HIGH);}
void PeppiDisplay::data8(uint8_t data){digitalWrite(PIN_DC,HIGH);digitalWrite(PIN_CS,LOW);spi_.transfer(data);digitalWrite(PIN_CS,HIGH);}
void PeppiDisplay::init(){
 pinMode(PIN_CS,OUTPUT);pinMode(PIN_DC,OUTPUT);pinMode(PIN_BL,OUTPUT);
 digitalWrite(PIN_CS,HIGH);digitalWrite(PIN_DC,HIGH);digitalWrite(PIN_BL,LOW);
 spi_.begin(PIN_SCK,PIN_MISO,PIN_MOSI,PIN_CS);spi_.beginTransaction(SPISettings(40000000,MSBFIRST,SPI_MODE0));
 command(0x01);delay(120);command(0x11);delay(120);command(0x3A);data8(0x55);command(0x36);data8(0x28);command(0x21);command(0x29);delay(20);
 _width=WIDTH;_height=HEIGHT;rotation=0;setTextWrap(false);fillScreen(0x0000);digitalWrite(PIN_BL,HIGH);
}
void PeppiDisplay::setRotation(uint8_t){rotation=0;_width=WIDTH;_height=HEIGHT;}
void PeppiDisplay::setBacklight(uint8_t value){
#if ESP_ARDUINO_VERSION_MAJOR >= 3
 static bool attached=false;if(!attached){ledcAttach(PIN_BL,5000,8);attached=true;}ledcWrite(PIN_BL,value);
#else
 static bool attached=false;if(!attached){ledcSetup(0,5000,8);ledcAttachPin(PIN_BL,0);attached=true;}ledcWrite(0,value);
#endif
}
uint8_t PeppiDisplay::scaleForFont(int font) const{switch(font){case 4:return 3;case 2:return 2;default:return 1;}}
void PeppiDisplay::setTextFont(uint8_t font){setFont(nullptr);setTextSize(scaleForFont(font));}
int16_t PeppiDisplay::textWidth(const String& value){int16_t x1,y1;uint16_t w,h;getTextBounds(value.c_str(),0,0,&x1,&y1,&w,&h);return (int16_t)w;}
int16_t PeppiDisplay::textWidth(const String& value,int font){const GFXfont* oldFont=gfxFont;uint8_t oldSizeX=textsize_x,oldSizeY=textsize_y;setFont(nullptr);setTextSize(scaleForFont(font));int16_t result=textWidth(value);setFont(oldFont);setTextSize(oldSizeX,oldSizeY);return result;}
int16_t PeppiDisplay::drawString(const String& value,int32_t x,int32_t y,int font){
 const GFXfont* oldFont=gfxFont;uint8_t oldSizeX=textsize_x,oldSizeY=textsize_y;if(font>0){setFont(nullptr);setTextSize(scaleForFont(font));}
 int16_t x1,y1;uint16_t w,h;getTextBounds(value.c_str(),0,0,&x1,&y1,&w,&h);int16_t cx=(int16_t)x,baseline=(int16_t)y;
 if(datum_==MC_DATUM){cx-=(int16_t)(x1+w/2);baseline-=(int16_t)(y1+h/2);}else if(datum_==TC_DATUM){cx-=(int16_t)(x1+w/2);baseline-=y1;}else{cx-=x1;baseline-=y1;}
 setCursor(cx,baseline);print(value);int16_t result=(int16_t)w;if(font>0){setFont(oldFont);setTextSize(oldSizeX,oldSizeY);}return result;
}
void PeppiDisplay::setAddrWindowRaw(int16_t x,int16_t y,int16_t w,int16_t h){
 const int16_t x2=x+w-1,y2=y+h-1;command(0x2A);digitalWrite(PIN_DC,HIGH);digitalWrite(PIN_CS,LOW);spi_.transfer16((uint16_t)x);spi_.transfer16((uint16_t)x2);digitalWrite(PIN_CS,HIGH);
 command(0x2B);digitalWrite(PIN_DC,HIGH);digitalWrite(PIN_CS,LOW);spi_.transfer16((uint16_t)y);spi_.transfer16((uint16_t)y2);digitalWrite(PIN_CS,HIGH);command(0x2C);
}
void PeppiDisplay::pushColor(uint16_t color,uint32_t count){uint8_t buf[256];const uint8_t hi=color>>8,lo=color&0xFF;for(size_t i=0;i<sizeof(buf);i+=2){buf[i]=hi;buf[i+1]=lo;}digitalWrite(PIN_DC,HIGH);digitalWrite(PIN_CS,LOW);while(count){const uint32_t n=count>128?128:count;spi_.writeBytes(buf,n*2);count-=n;}digitalWrite(PIN_CS,HIGH);}
void PeppiDisplay::writeFillRect(int16_t x,int16_t y,int16_t w,int16_t h,uint16_t color){if(w<=0||h<=0)return;if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x>=width()||y>=height())return;if(x+w>width())w=width()-x;if(y+h>height())h=height()-y;if(w<=0||h<=0)return;setAddrWindowRaw(x,y,w,h);pushColor(color,(uint32_t)w*(uint32_t)h);}
void PeppiDisplay::drawPixel(int16_t x,int16_t y,uint16_t color){writePixel(x,y,color);}
void PeppiDisplay::writePixel(int16_t x,int16_t y,uint16_t color){if(x<0||y<0||x>=width()||y>=height())return;setAddrWindowRaw(x,y,1,1);pushColor(color,1);}
void PeppiDisplay::writeFastHLine(int16_t x,int16_t y,int16_t w,uint16_t color){writeFillRect(x,y,w,1,color);}
void PeppiDisplay::writeFastVLine(int16_t x,int16_t y,int16_t h,uint16_t color){writeFillRect(x,y,1,h,color);}
void PeppiDisplay::pushImage(int16_t x,int16_t y,uint16_t w,uint16_t h,const uint16_t* pixels){
 if(!pixels||w==0||h==0)return;if(x<0||y<0||x+w>width()||y+h>height()){for(uint16_t yy=0;yy<h;++yy)for(uint16_t xx=0;xx<w;++xx)drawPixel(x+xx,y+yy,pixels[(uint32_t)yy*w+xx]);return;}
 setAddrWindowRaw(x,y,w,h);digitalWrite(PIN_DC,HIGH);digitalWrite(PIN_CS,LOW);const uint32_t n=(uint32_t)w*h;uint8_t buf[512];uint32_t i=0;
 while(i<n){const uint32_t chunk=(n-i)>256?256:(n-i);for(uint32_t k=0;k<chunk;++k){const uint16_t v=pixels[i+k];buf[2*k]=v>>8;buf[2*k+1]=v&0xFF;}spi_.writeBytes(buf,chunk*2);i+=chunk;}digitalWrite(PIN_CS,HIGH);
}
void PeppiDisplay::drawEllipse(int16_t x0,int16_t y0,int16_t rx,int16_t ry,uint16_t color){if(rx<=0||ry<=0)return;for(int16_t x=-rx;x<=rx;++x){float q=1.0f-(float(x)*float(x))/(float(rx)*float(rx));if(q<0)continue;int16_t y=(int16_t)lroundf(float(ry)*sqrtf(q));drawPixel(x0+x,y0+y,color);drawPixel(x0+x,y0-y,color);}}
void PeppiDisplay::fillEllipse(int16_t x0,int16_t y0,int16_t rx,int16_t ry,uint16_t color){if(rx<=0||ry<=0)return;for(int16_t y=-ry;y<=ry;++y){float q=1.0f-(float(y)*float(y))/(float(ry)*float(ry));if(q<0)continue;int16_t x=(int16_t)lroundf(float(rx)*sqrtf(q));drawFastHLine(x0-x,y0+y,x*2+1,color);}}
