from pathlib import Path
import re

sketch = next(Path('sketch').glob('Weihnachtsuhr_v*_ES3C28P'))
main = sketch / 'main.cpp'
web = sketch / 'WebPage.h'

(sketch / 'lv_conf.h').write_text(r'''#ifndef LV_CONF_H
#define LV_CONF_H
#define LV_COLOR_DEPTH 16
#define LV_MEM_SIZE (192U * 1024U)
#define LV_USE_LOG 0
#define LV_USE_OS LV_OS_NONE
#define LV_FONT_MONTSERRAT_12 1
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 1
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_24 1
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_32 1
#define LV_FONT_MONTSERRAT_36 1
#define LV_FONT_MONTSERRAT_40 1
#define LV_FONT_MONTSERRAT_44 1
#define LV_FONT_MONTSERRAT_48 1
#define LV_FONT_DEFAULT &lv_font_montserrat_14
#endif
''')

s = main.read_text()
if '#include <lvgl.h>' not in s:
    s = s.replace('#include <Arduino.h>\n', '#include <Arduino.h>\n#include <lvgl.h>\n#include <esp_heap_caps.h>\n')

anchor = 'void render(){\n'
if anchor not in s:
    raise SystemExit('render anchor not found')

ui = r'''
// ---- Peppi v5.0 / LVGL UI -------------------------------------------------
// SquareLine-style structure: persistent screen, components, styles and events.
struct V5Palette { uint32_t top,bottom,accent,accent2; };
lv_display_t* v5Display=nullptr;
lv_indev_t* v5Input=nullptr;
lv_obj_t* v5Screen=nullptr;
uint8_t* v5BufA=nullptr;
uint8_t* v5BufB=nullptr;
bool v5Ready=false;
volatile uint32_t v5FlushCount=0;

V5Palette v5Palette(){
  switch(eventSettings.mode){
    case countdown::Silvester:return {0x090A12,0x20170A,0xF6C85F,0xFFE6A3};
    case countdown::Birthday:return {0x120A26,0x301752,0xB58CFF,0xE9DDFF};
    case countdown::Valentines:return {0x210914,0x54162F,0xFF6FA5,0xFFD0E1};
    case countdown::Easter:return {0x071A17,0x103B31,0x6EE7B7,0xD1FAE5};
    case countdown::Vacation:return {0x071827,0x0C3D58,0x5CC8FF,0xD3F1FF};
    case countdown::Halloween:return {0x1B0C05,0x4A1C08,0xFF9B42,0xFFE0BF};
    case countdown::Nikolaus:return {0x210708,0x4C1116,0xFF7B82,0xFFE0E2};
    default:return {0x13080B,0x3B1018,0xEF6670,0xFFE1D5};
  }
}

static lv_color_t v5c(uint32_t rgb){return lv_color_hex(rgb);}
static void v5Flush(lv_display_t* disp,const lv_area_t* area,uint8_t* px){
  const uint16_t w=uint16_t(area->x2-area->x1+1),h=uint16_t(area->y2-area->y1+1);
  // PeppiDisplay::pushImage() consumes native uint16_t RGB565 values and
  // SPI::transfer16() sends them MSB first. Do NOT byte-swap the LVGL buffer.
  lcd.pushImage(area->x1,area->y1,w,h,reinterpret_cast<const uint16_t*>(px));
  ++v5FlushCount;
  lv_display_flush_ready(disp);
}
static void v5Touch(lv_indev_t*,lv_indev_data_t* data){
  touch::Point p;
  if(touch::raw(p)){
    data->state=LV_INDEV_STATE_PRESSED;data->point.x=p.x;data->point.y=p.y;
  }else data->state=LV_INDEV_STATE_RELEASED;
}
static void v5Base(lv_obj_t* o,uint32_t color,uint8_t opa=255,int radius=18){
  lv_obj_set_style_bg_color(o,v5c(color),0);lv_obj_set_style_bg_opa(o,opa,0);
  lv_obj_set_style_border_width(o,0,0);lv_obj_set_style_radius(o,radius,0);
  lv_obj_set_style_pad_all(o,0,0);lv_obj_remove_flag(o,LV_OBJ_FLAG_SCROLLABLE);
}
static void v5Label(lv_obj_t* parent,const String& text,int x,int y,int w,int h,const lv_font_t* font,uint32_t color,lv_text_align_t align=LV_TEXT_ALIGN_CENTER){
  lv_obj_t* l=lv_label_create(parent);lv_label_set_text(l,text.c_str());lv_obj_set_pos(l,x,y);lv_obj_set_size(l,w,h);
  lv_obj_set_style_text_font(l,font,0);lv_obj_set_style_text_color(l,v5c(color),0);lv_obj_set_style_text_align(l,align,0);
  lv_label_set_long_mode(l,LV_LABEL_LONG_DOT);
}
static lv_obj_t* v5Button(lv_obj_t* parent,const char* symbol,const String& text,int x,int y,int w,int h,uint32_t fill,uint32_t accent,lv_event_cb_t cb){
  lv_obj_t* b=lv_button_create(parent);lv_obj_set_pos(b,x,y);lv_obj_set_size(b,w,h);
  v5Base(b,fill,230,15);lv_obj_set_style_border_width(b,1,0);lv_obj_set_style_border_color(b,v5c(accent),0);lv_obj_set_style_border_opa(b,95,0);
  lv_obj_set_style_bg_color(b,v5c(accent),LV_STATE_PRESSED);lv_obj_set_style_bg_opa(b,125,LV_STATE_PRESSED);
  if(cb)lv_obj_add_event_cb(b,cb,LV_EVENT_CLICKED,nullptr);
  String caption=String(symbol);if(text.length()){caption+=' ';caption+=text;}
  lv_obj_t* l=lv_label_create(b);lv_label_set_text(l,caption.c_str());lv_obj_center(l);lv_obj_set_style_text_font(l,&lv_font_montserrat_14,0);lv_obj_set_style_text_color(l,v5c(0xFFF8F4),0);
  return b;
}
static String v5Track(){
  String title=music::title();if(!title.length())title=tr("Keine Musik ausgewaehlt","Nie wybrano muzyki","No track selected");
  int dot=title.lastIndexOf('.');if(dot>0)title=title.substring(0,dot);return title;
}
static String v5Clock(){tm t{};if(!localDate(t))return "--:--";char b[6];snprintf(b,sizeof(b),"%02d:%02d",t.tm_hour,t.tm_min);return b;}
static String v5Date(){tm t{};if(!localDate(t))return "";char b[16];snprintf(b,sizeof(b),"%02d.%02d.%04d",t.tm_mday,t.tm_mon+1,t.tm_year+1900);return b;}

static void v5Back(lv_event_t*){configScreen=false;playerScreen=false;forceFull=true;dirty=true;}
static void v5OpenPlayer(lv_event_t*){configScreen=false;playerScreen=true;forceFull=true;dirty=true;}
static void v5OpenSetup(lv_event_t*){openAP();playerScreen=false;forceFull=true;dirty=true;}
static void v5Prev(lv_event_t*){music::setCategory(musicCategory());music::previous(sdReady);dirty=true;forceFull=true;}
static void v5Next(lv_event_t*){music::setCategory(musicCategory());music::next(sdReady);dirty=true;forceFull=true;}
static void v5Play(lv_event_t*){toggleMusic();dirty=true;forceFull=true;}
static void v5VolumeChanged(lv_event_t* e){music::setVolume((unsigned)lv_slider_get_value(lv_event_get_target_obj(e)));}
static void v5VolumeSave(lv_event_t*){Preferences p;if(p.begin("peppi",false)){p.putUChar("volume",(uint8_t)music::volume.load());p.end();}}

static void v5Root(){
  lv_obj_clean(v5Screen);V5Palette p=v5Palette();
  lv_obj_set_style_bg_color(v5Screen,v5c(p.top),0);lv_obj_set_style_bg_grad_color(v5Screen,v5c(p.bottom),0);
  lv_obj_set_style_bg_grad_dir(v5Screen,LV_GRAD_DIR_VER,0);lv_obj_set_style_bg_opa(v5Screen,LV_OPA_COVER,0);
  lv_obj_set_style_pad_all(v5Screen,0,0);lv_obj_remove_flag(v5Screen,LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_t* glow=lv_obj_create(v5Screen);v5Base(glow,p.accent,38,LV_RADIUS_CIRCLE);lv_obj_set_size(glow,170,170);lv_obj_set_pos(glow,205,-88);
}
static void v5Top(const String& title,bool back){
  V5Palette p=v5Palette();
  if(back)v5Button(v5Screen,LV_SYMBOL_LEFT,"",12,10,38,34,0x171820,p.accent,v5Back);
  lv_obj_t* chip=lv_obj_create(v5Screen);v5Base(chip,0x171820,205,13);lv_obj_set_pos(chip,back?58:12,10);lv_obj_set_size(chip,back?174:184,34);
  v5Label(chip,title,10,7,back?154:164,20,&lv_font_montserrat_14,0xFFF8F4,LV_TEXT_ALIGN_LEFT);
  lv_obj_t* time=lv_obj_create(v5Screen);v5Base(time,0x171820,180,13);lv_obj_set_pos(time,240,10);lv_obj_set_size(time,68,34);
  v5Label(time,v5Clock(),0,7,68,20,&lv_font_montserrat_14,p.accent2);
}
static void v5Home(){
  v5Root();V5Palette p=v5Palette();v5Top(eventTitle(),false);
  tm t{};bool valid=localDate(t);String big="--",kicker=tr("ZEIT FEHLT","BRAK CZASU","TIME NEEDED"),sub=v5Date();
  bool celebration=false;
  if(countdown::configured(eventSettings)&&valid){
    auto r=countdown::calculate({t.tm_year+1900,t.tm_mon+1,t.tm_mday},eventSettings);
    if(r.today||r.started){celebration=true;big="";kicker=r.started?tr("URLAUB GESTARTET","URLOP ROZPOCZETY","VACATION STARTED"):tr("HEUTE","DZISIAJ","TODAY");sub=eventTitle();}
    else {big=String(r.days);kicker=r.days==1?tr("TAG BIS","DZIEN DO","DAY UNTIL"):tr("TAGE BIS","DNI DO","DAYS UNTIL");sub=eventTitle();}
  }else if(!countdown::configured(eventSettings)){kicker=tr("DATUM EINSTELLEN","USTAW DATE","SET EVENT DATE");sub=tr("im Web-Panel","w panelu WWW","in the web panel");}
  lv_obj_t* hero=lv_obj_create(v5Screen);v5Base(hero,0x0A0B10,205,22);lv_obj_set_pos(hero,14,54);lv_obj_set_size(hero,292,124);
  lv_obj_set_style_border_width(hero,1,0);lv_obj_set_style_border_color(hero,v5c(p.accent),0);lv_obj_set_style_border_opa(hero,65,0);
  lv_obj_t* bar=lv_obj_create(hero);v5Base(bar,p.accent,255,4);lv_obj_set_pos(bar,0,20);lv_obj_set_size(bar,4,84);
  v5Label(hero,kicker,18,13,256,20,&lv_font_montserrat_12,p.accent2,LV_TEXT_ALIGN_LEFT);
  if(celebration){v5Label(hero,sub,18,47,256,36,&lv_font_montserrat_24,0xFFF8F4,LV_TEXT_ALIGN_LEFT);}
  else {v5Label(hero,big,18,31,104,58,&lv_font_montserrat_48,0xFFF8F4,LV_TEXT_ALIGN_LEFT);v5Label(hero,sub,126,48,148,28,&lv_font_montserrat_18,0xFFF8F4,LV_TEXT_ALIGN_LEFT);}
  if(v5Date().length())v5Label(hero,v5Date(),18,93,256,18,&lv_font_montserrat_12,0xAEB2BE,LV_TEXT_ALIGN_LEFT);
  v5Button(v5Screen,LV_SYMBOL_WIFI,tr("Setup","Ustawienia","Setup"),14,190,140,38,0x12141B,p.accent,v5OpenSetup);
  v5Button(v5Screen,LV_SYMBOL_AUDIO,tr("Musik","Muzyka","Music"),166,190,140,38,0x12141B,p.accent,v5OpenPlayer);
}
static void v5Player(){
  v5Root();V5Palette p=v5Palette();v5Top(eventTitle(),true);
  lv_obj_t* card=lv_obj_create(v5Screen);v5Base(card,0x0A0B10,215,20);lv_obj_set_pos(card,18,56);lv_obj_set_size(card,284,88);
  lv_obj_set_style_border_width(card,1,0);lv_obj_set_style_border_color(card,v5c(p.accent),0);lv_obj_set_style_border_opa(card,60,0);
  v5Label(card,tr("JETZT","TERAZ","NOW PLAYING"),16,12,252,16,&lv_font_montserrat_12,p.accent2,LV_TEXT_ALIGN_LEFT);
  v5Label(card,v5Track(),16,36,252,40,&lv_font_montserrat_18,0xFFF8F4,LV_TEXT_ALIGN_LEFT);
  v5Label(v5Screen,LV_SYMBOL_VOLUME_MAX,20,157,24,20,&lv_font_montserrat_16,p.accent2);
  lv_obj_t* slider=lv_slider_create(v5Screen);lv_obj_set_pos(slider,49,160);lv_obj_set_size(slider,244,8);lv_slider_set_range(slider,0,100);lv_slider_set_value(slider,(int)music::volume.load(),LV_ANIM_OFF);
  lv_obj_set_style_bg_color(slider,v5c(0x343741),LV_PART_MAIN);lv_obj_set_style_bg_opa(slider,255,LV_PART_MAIN);lv_obj_set_style_radius(slider,LV_RADIUS_CIRCLE,LV_PART_MAIN);
  lv_obj_set_style_bg_color(slider,v5c(p.accent),LV_PART_INDICATOR);lv_obj_set_style_bg_opa(slider,255,LV_PART_INDICATOR);
  lv_obj_set_style_bg_color(slider,v5c(0xFFF8F4),LV_PART_KNOB);lv_obj_set_style_pad_all(slider,5,LV_PART_KNOB);
  lv_obj_add_event_cb(slider,v5VolumeChanged,LV_EVENT_VALUE_CHANGED,nullptr);lv_obj_add_event_cb(slider,v5VolumeSave,LV_EVENT_RELEASED,nullptr);
  v5Button(v5Screen,LV_SYMBOL_PREV,"",60,187,54,42,0x13151D,p.accent,v5Prev);
  v5Button(v5Screen,music::state==music::Playing?LV_SYMBOL_PAUSE:LV_SYMBOL_PLAY,"",126,181,68,50,p.accent,p.accent2,v5Play);
  v5Button(v5Screen,LV_SYMBOL_NEXT,"",206,187,54,42,0x13151D,p.accent,v5Next);
}
static void v5Config(){
  v5Root();V5Palette p=v5Palette();v5Top("Wi-Fi Setup",true);
  lv_obj_t* panel=lv_obj_create(v5Screen);v5Base(panel,0x0A0B10,215,20);lv_obj_set_pos(panel,16,56);lv_obj_set_size(panel,288,166);
  lv_obj_set_style_border_width(panel,1,0);lv_obj_set_style_border_color(panel,v5c(p.accent),0);lv_obj_set_style_border_opa(panel,55,0);
  const String labels[3]={tr("NETZWERK","SIEC","NETWORK"),tr("PASSWORT","HASLO","PASSWORD"),tr("ADRESSE","ADRES","ADDRESS")};
  const String vals[3]={apName,adminPassword,"192.168.4.1"};
  for(int i=0;i<3;++i){
    int y=10+i*43;lv_obj_t* row=lv_obj_create(panel);v5Base(row,0x171920,190,12);lv_obj_set_pos(row,10,y);lv_obj_set_size(row,268,36);
    v5Label(row,labels[i],12,4,80,15,&lv_font_montserrat_12,p.accent2,LV_TEXT_ALIGN_LEFT);
    v5Label(row,vals[i],94,4,160,24,&lv_font_montserrat_14,0xFFF8F4,LV_TEXT_ALIGN_RIGHT);
  }
  v5Label(panel,tr("Mit WLAN verbinden und 192.168.4.1 oeffnen","Polacz z Wi-Fi i otworz 192.168.4.1","Connect to Wi-Fi and open 192.168.4.1"),12,142,264,16,&lv_font_montserrat_12,0x9CA3AF);
}
static bool v5LvActive(){return eventSettings.mode!=countdown::PhotoAlbum||configScreen||playerScreen;}
static void v5UiBegin(){
  lv_init();
  const size_t bytes=320U*34U*2U;
  v5BufA=(uint8_t*)heap_caps_malloc(bytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  v5BufB=(uint8_t*)heap_caps_malloc(bytes,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
  if(!v5BufA)v5BufA=(uint8_t*)malloc(bytes);if(!v5BufB)v5BufB=(uint8_t*)malloc(bytes);
  if(!v5BufA||!v5BufB){Serial.println("LVGL buffer allocation failed");return;}
  v5Display=lv_display_create(320,240);lv_display_set_default(v5Display);lv_display_set_color_format(v5Display,LV_COLOR_FORMAT_RGB565);lv_display_set_flush_cb(v5Display,v5Flush);
  lv_display_set_buffers(v5Display,v5BufA,v5BufB,bytes,LV_DISPLAY_RENDER_MODE_PARTIAL);
  v5Input=lv_indev_create();lv_indev_set_type(v5Input,LV_INDEV_TYPE_POINTER);lv_indev_set_read_cb(v5Input,v5Touch);
  v5Screen=lv_obj_create(nullptr);lv_obj_set_size(v5Screen,320,240);lv_screen_load(v5Screen);v5Ready=true;
  lv_obj_invalidate(v5Screen);
}

'''
s = s.replace(anchor, ui + anchor, 1)

s, n = re.subn(r'void render\(\)\{.*?\n\}\nvoid pollTouch\(\) \{.*?\n\}\nvoid noCache\(\)', r'''void render(){
  if(!v5Ready)return;
  if(configScreen)v5Config();
  else if(playerScreen)v5Player();
  else if(eventSettings.mode==countdown::PhotoAlbum){drawPhotoAlbum();}
  else v5Home();
  // Force the first frame immediately; do not wait for LVGL's refresh timer.
  if(v5LvActive()){lv_obj_invalidate(v5Screen);lv_refr_now(v5Display);}
  forceFull=false;dirty=false;lastRender=millis();
}
void pollTouch(){ /* LVGL reads FT6336G through v5Touch(). */ }
void noCache()''', s, flags=re.S)
if n != 1:
    raise SystemExit(f'render/touch replacement count={n}')

old = 'touch::begin();music::begin();mountSD();'
new = 'touch::begin();music::begin();mountSD();v5UiBegin();'
if old not in s: raise SystemExit('setup init anchor missing')
s = s.replace(old,new,1)
s = s.replace('Weihnachtsuhr v4.2 / ES3C28P. Hold BOOT for 3s after startup to open setup.',
              'Weihnachtsuhr v5.0.1 LVGL / ES3C28P. Hold BOOT for 3s after startup to open setup.')

old_loop = '''void loop() {
  if(!ready){delay(50);return;}server.handleClient();if(apActive)dns.processNextRequest();wifiTick();pollBoot();pollTouch();if(eventSettings.mode!=countdown::PhotoAlbum){music::setCategory(musicCategory());music::loop(sdReady);}'''
new_loop = '''void loop() {
  static uint32_t lvLast=millis();uint32_t lvNow=millis();if(v5Ready){lv_tick_inc(lvNow-lvLast);lvLast=lvNow;}
  if(!ready){delay(50);return;}server.handleClient();if(apActive)dns.processNextRequest();wifiTick();pollBoot();if(eventSettings.mode!=countdown::PhotoAlbum){music::setCategory(musicCategory());music::loop(sdReady);}'''
if old_loop not in s: raise SystemExit('loop anchor missing')
s = s.replace(old_loop,new_loop,1)
s = s.replace('  if(dirty&&millis()-lastRender>100)render();delay(2);\n}',
              '  if(dirty&&millis()-lastRender>70)render();if(v5Ready&&v5LvActive())lv_timer_handler();delay(2);\n}',1)

main.write_text(s)

w = web.read_text()
w = w.replace('v4.8 ES3C28P','v5.0.1 LVGL ES3C28P').replace('v4.3 ES3C28P','v5.0.1 LVGL ES3C28P')
web.write_text(w)
