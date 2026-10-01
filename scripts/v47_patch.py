from pathlib import Path
import re

p = next(Path("sketch").glob("Weihnachtsuhr_v*_ES3C28P/main.cpp"))
s = p.read_text()

player = r'''void drawPlayerDynamic(){
  const uint16_t PANEL=0x0861, TRACK=0x2965;
  String title=music::title();
  if(!title.length())title=tr("Keine Musik gewaehlt","Nie wybrano muzyki","No track selected");
  int dot=title.lastIndexOf('.');if(dot>0)title=title.substring(0,dot);

  String line1=title,line2="";
  if(title.length()>27){
    int split=title.lastIndexOf(' ',27);
    if(split<12)split=27;
    line1=title.substring(0,split);
    line2=title.substring(split+1);
    if(line2.length()>27)line2=line2.substring(0,24)+"...";
  }

  lcd.fillRoundRect(30,76,260,68,14,PANEL);
  screen::text(lcd,tr("JETZT","TERAZ","NOW PLAYING"),160,91,&FreeSans9pt7b,screen::GOLD,238);
  screen::text(lcd,line1,160,116,&FreeSans9pt7b,screen::CREAM,238);
  if(line2.length())screen::text(lcd,line2,160,135,&FreeSans9pt7b,screen::CREAM,238);

  lcd.fillRoundRect(34,150,252,28,14,PANEL);
  screen::text(lcd,"VOL",54,164,&FreeSans9pt7b,screen::CREAM,38);
  const int x1=82,x2=260,y=164;
  lcd.fillRoundRect(x1,y-2,x2-x1,4,2,TRACK);
  int vx=x1+(int)(music::volume.load()*(x2-x1)/100U);
  if(vx>x1)lcd.fillRoundRect(x1,y-2,vx-x1,4,2,screen::GOLD);
  lcd.fillCircle(vx,y,6,screen::CREAM);
  lcd.drawCircle(vx,y,6,screen::GOLD);

  lcd.fillCircle(78,210,18,PANEL);
  lcd.drawCircle(78,210,18,screen::GOLD);
  lcd.fillTriangle(73,202,73,218,62,210,screen::CREAM);
  lcd.fillRect(60,202,3,16,screen::CREAM);

  lcd.fillCircle(160,207,24,PANEL);
  lcd.drawCircle(160,207,24,screen::GOLD);
  if(music::state==music::Playing){
    lcd.fillRoundRect(151,196,6,22,2,screen::CREAM);
    lcd.fillRoundRect(164,196,6,22,2,screen::CREAM);
  } else {
    lcd.fillTriangle(154,195,154,219,174,207,screen::CREAM);
  }

  lcd.fillCircle(242,210,18,PANEL);
  lcd.drawCircle(242,210,18,screen::GOLD);
  lcd.fillTriangle(247,202,247,218,258,210,screen::CREAM);
  lcd.fillRect(258,202,3,16,screen::CREAM);
}

void drawPlayer(){
  music::setCategory(musicCategory());
  drawArtwork(playerArtwork());
  const uint16_t PANEL=0x0861;
  lcd.fillCircle(24,24,16,PANEL);
  lcd.drawCircle(24,24,16,screen::GOLD);
  lcd.drawFastHLine(17,24,13,screen::CREAM);
  lcd.drawLine(17,24,23,18,screen::CREAM);
  lcd.drawLine(17,24,23,30,screen::CREAM);
  drawPlayerDynamic();
}'''

s, n = re.subn(r'void drawPlayerDynamic\(\)\{.*?\n\}\n\nvoid drawPlayer\(\)\{.*?\n\}', player, s, flags=re.S)
if n != 1:
    raise SystemExit(f"player replacement count={n}")

s = s.replace(
    'if(playerScreen && p.y>=146 && p.y<=184 && p.x>=76 && p.x<=275){',
    'if(playerScreen && p.y>=146 && p.y<=181 && p.x>=72 && p.x<=270){'
)
s = s.replace(
    'int level=constrain((p.x-88)*100/(257-88),0,100);',
    'int level=constrain((p.x-82)*100/(260-82),0,100);'
)
s = s.replace('if(p.x<58&&p.y<60)', 'if(p.x<48&&p.y<48)')

p.write_text(s)
