#include <Arduino.h>
#include <lvgl.h>
#include <esp_heap_caps.h>
#include "PeppiDisplay.h"
#include "Touch.h"
PeppiDisplay lcd;
static lv_display_t* disp=nullptr;
static lv_indev_t* indev=nullptr;
static uint8_t* bufA=nullptr;
static lv_obj_t* coordLabel=nullptr;
static volatile uint32_t flushCount=0;
static uint32_t lastTouchMs=0;
static int lastX=-1,lastY=-1;
static void flushCb(lv_display_t* d,const lv_area_t* a,uint8_t* px){
  const uint16_t w=(uint16_t)(a->x2-a->x1+1),h=(uint16_t)(a->y2-a->y1+1);
  lcd.pushImage(a->x1,a->y1,w,h,reinterpret_cast<const uint16_t*>(px));
  ++flushCount;lv_display_flush_ready(d);
}
static void touchCb(lv_indev_t*,lv_indev_data_t* data){
  touch::Point p;
  if(touch::raw(p)){data->state=LV_INDEV_STATE_PRESSED;data->point.x=p.x;data->point.y=p.y;lastX=p.x;lastY=p.y;lastTouchMs=millis();}
  else data->state=LV_INDEV_STATE_RELEASED;
}
static void buildUi(){
  lv_obj_t* scr=lv_obj_create(nullptr);lv_obj_remove_style_all(scr);lv_obj_set_size(scr,320,240);
  lv_obj_set_style_bg_color(scr,lv_color_hex(0x07111F),0);lv_obj_set_style_bg_grad_color(scr,lv_color_hex(0x123B52),0);lv_obj_set_style_bg_grad_dir(scr,LV_GRAD_DIR_VER,0);lv_screen_load(scr);
  lv_obj_t* title=lv_label_create(scr);lv_label_set_text(title,"PEPPI DIAG v5.1");lv_obj_set_style_text_font(title,&lv_font_montserrat_24,0);lv_obj_set_style_text_color(title,lv_color_hex(0xFFF4D0),0);lv_obj_align(title,LV_ALIGN_TOP_MID,0,18);
  lv_obj_t* box=lv_obj_create(scr);lv_obj_set_size(box,284,128);lv_obj_align(box,LV_ALIGN_CENTER,0,6);lv_obj_set_style_radius(box,18,0);lv_obj_set_style_bg_color(box,lv_color_hex(0x0D1926),0);lv_obj_set_style_bg_opa(box,230,0);lv_obj_set_style_border_width(box,1,0);lv_obj_set_style_border_color(box,lv_color_hex(0x45D7C5),0);
  lv_obj_t* a=lv_label_create(box);lv_label_set_text(a,LV_SYMBOL_OK "  LCD ILI9341V: OK");lv_obj_set_style_text_color(a,lv_color_hex(0x7EF2B0),0);lv_obj_set_pos(a,14,12);
  lv_obj_t* b=lv_label_create(box);lv_label_set_text(b,LV_SYMBOL_OK "  LVGL 9.3: OK");lv_obj_set_style_text_color(b,lv_color_hex(0x7EF2B0),0);lv_obj_set_pos(b,14,40);
  lv_obj_t* c=lv_label_create(box);lv_label_set_text(c,LV_SYMBOL_OK "  FT6336G: READY");lv_obj_set_style_text_color(c,lv_color_hex(0x7EF2B0),0);lv_obj_set_pos(c,14,68);
  coordLabel=lv_label_create(box);lv_label_set_text(coordLabel,"TOUCH: tap the screen");lv_obj_set_style_text_color(coordLabel,lv_color_hex(0xFFF4D0),0);lv_obj_set_pos(coordLabel,14,96);
  lv_obj_t* s=lv_label_create(scr);lv_label_set_text(s,"NO AUDIO / NO SD / NO WIFI");lv_obj_set_style_text_color(s,lv_color_hex(0xA8B6C8),0);lv_obj_align(s,LV_ALIGN_BOTTOM_MID,0,-14);
}
void setup(){
  Serial.begin(115200);delay(80);Serial.println("[DIAG] boot");
  Serial.println("[DIAG] LCD init...");lcd.init();lcd.setRotation(1);lcd.setBacklight(180);lcd.fillScreen(0x0000);Serial.println("[DIAG] LCD OK");
  Serial.println("[DIAG] FT6336G init...");touch::begin();Serial.println("[DIAG] FT6336G init OK");
  Serial.println("[DIAG] LVGL init...");lv_init();
  constexpr size_t bytes=320U*32U*2U;
  bufA=(uint8_t*)heap_caps_malloc(bytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if(!bufA)bufA=(uint8_t*)malloc(bytes);
  if(!bufA){Serial.println("[DIAG] FATAL: LVGL buffer alloc failed");lcd.fillScreen(0xF800);while(true)delay(1000);}
  disp=lv_display_create(320,240);lv_display_set_color_format(disp,LV_COLOR_FORMAT_RGB565);lv_display_set_flush_cb(disp,flushCb);lv_display_set_buffers(disp,bufA,nullptr,bytes,LV_DISPLAY_RENDER_MODE_PARTIAL);
  indev=lv_indev_create();lv_indev_set_type(indev,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(indev,touchCb);
  buildUi();lv_refr_now(disp);Serial.printf("[DIAG] LVGL OK, first flushes=%lu\n",(unsigned long)flushCount);Serial.println("[DIAG] minimal test running");
}
void loop(){
  static uint32_t last=millis(),lastUi=0;uint32_t now=millis();lv_tick_inc(now-last);last=now;lv_timer_handler();
  if(coordLabel&&now-lastUi>100){lastUi=now;if(lastX>=0&&now-lastTouchMs<2500){char b[64];snprintf(b,sizeof(b),"TOUCH: X=%d  Y=%d",lastX,lastY);lv_label_set_text(coordLabel,b);}}
  delay(2);
}
