from pathlib import Path
import re

sketch = next(Path("sketch").glob("Weihnachtsuhr_v*_ES3C28P"))
countdown = sketch / "Countdown.h"
main = sketch / "main.cpp"
web = sketch / "WebPage.h"

# ---- Countdown modes ----
s = countdown.read_text()
s = s.replace(
    'enum Mode:uint8_t {Christmas,Birthday,Valentines,Easter,Vacation,Halloween};',
    'enum Mode:uint8_t {Christmas,Birthday,Valentines,Easter,Vacation,Halloween,Silvester,Nikolaus,PhotoAlbum};'
)
s = s.replace(
    'const char* names[]={"christmas","birthday","valentines","easter","vacation","halloween"};return mode<=Halloween?names[mode]:names[0];',
    'const char* names[]={"christmas","birthday","valentines","easter","vacation","halloween","silvester","nikolaus","photoalbum"};return mode<=PhotoAlbum?names[mode]:names[0];'
)
s = s.replace('for(int i=0;i<=Halloween;++i)', 'for(int i=0;i<=PhotoAlbum;++i)')
s = s.replace('c.magic==0x43445431&&c.mode<=Halloween', 'c.magic==0x43445431&&c.mode<=PhotoAlbum')
s = s.replace(
    ' case Christmas:r.target={y+(now.month==12&&now.day>26),12,24};break;',
    ' case Christmas:r.target={y+(now.month==12&&now.day>26),12,24};break;\n case Silvester:r.target={y,12,31};break;\n case Nikolaus:r.target={y,12,6};break;\n case PhotoAlbum:r.configured=true;r.today=false;r.started=false;r.days=0;return r;'
)
countdown.write_text(s)

# ---- Firmware / SD photo album ----
s = main.read_text()
s = s.replace(
    '    case countdown::Halloween:return "Halloween";\n    default:return tr("Weihnachten","Swieta","Christmas");',
    '    case countdown::Halloween:return "Halloween";\n    case countdown::Silvester:return tr("Silvester","Sylwester","New Year");\n    case countdown::Nikolaus:return tr("Nikolaus","Mikolaj","St. Nicholas");\n    case countdown::PhotoAlbum:return tr("Fotoalbum","Fotoalbum","Photo album");\n    default:return tr("Weihnachten","Swieta","Christmas");'
)

start = s.index('String musicCategory(){')
end = s.index('\nString playerArtwork(){', start)
s = s[:start] + '''String musicCategory(){
  switch(eventSettings.mode){
    case countdown::Birthday:return "birthday";
    case countdown::Valentines:return "valentines";
    case countdown::Easter:return "easter";
    case countdown::Vacation:return "vacation";
    case countdown::Halloween:return "halloween";
    case countdown::Silvester:return "silvester";
    case countdown::Nikolaus:return "nikolaus";
    default:return "christmas";
  }
}
''' + s[end+1:]

s = s.replace(
    '  static const char* allowed[]={"christmas","silvester","birthday","halloween","easter","valentines","vacation","krampus","nikolaus","grincz","normal"};',
    '  static const char* allowed[]={"christmas","silvester","birthday","halloween","easter","valentines","vacation","nikolaus"};'
)
s = s.replace(
    'String mediaImagePath(const String& c){return String("/media/images/")+c+".jpg";}',
    'String mediaImagePath(const String& c){return String("/media/images/")+c+".jpg";}\nString albumDir(){return "/media/album";}'
)
s = s.replace(
    '  if(!SD_MMC.exists("/media/images"))SD_MMC.mkdir("/media/images");',
    '  if(!SD_MMC.exists("/media/images"))SD_MMC.mkdir("/media/images");\n  if(!SD_MMC.exists(albumDir()))SD_MMC.mkdir(albumDir());'
)

s = s.replace(
    '  if(eventSettings.mode==countdown::Halloween)return "halloween";\n  tm t{};',
    '  if(eventSettings.mode==countdown::Halloween)return "halloween";\n  if(eventSettings.mode==countdown::Silvester)return "silvester";\n  if(eventSettings.mode==countdown::Nikolaus)return "nikolaus";\n  tm t{};'
)

# Christmas-only special images.
s = s.replace(
    '  if(scene==peppi::Scene::Christmas)return "christmas";\n  if(scene==peppi::Scene::Krampus)return "krampus";\n  if(scene==peppi::Scene::Nikolaus)return "nikolaus";\n  if(m==12&&d==27)return "grincz";\n  if(m==1&&d==6)return "grincz_alt";',
    '  if(m==12&&d==5)return "krampus";\n  if(m==12&&d==6)return "nikolaus";\n  if((m==12&&d==24)||(m==1&&d==6))return "grincz";\n  if(scene==peppi::Scene::Christmas)return "christmas";'
)

album_helpers = r'''int albumCount(){
  if(!sdReady||!SD_MMC.exists(albumDir()))return 0;
  File dir=SD_MMC.open(albumDir());if(!dir||!dir.isDirectory())return 0;
  int n=0;
  for(File f=dir.openNextFile();f;f=dir.openNextFile()){
    if(f.isDirectory())continue;
    String x=f.name();x.toLowerCase();
    if(x.endsWith(".jpg")||x.endsWith(".jpeg"))++n;
  }
  return n;
}
String albumPathAt(int wanted){
  if(!sdReady||!SD_MMC.exists(albumDir()))return "";
  File dir=SD_MMC.open(albumDir());if(!dir||!dir.isDirectory())return "";
  int i=0;
  for(File f=dir.openNextFile();f;f=dir.openNextFile()){
    if(f.isDirectory())continue;
    String x=f.name(),l=x;l.toLowerCase();
    if(!(l.endsWith(".jpg")||l.endsWith(".jpeg")))continue;
    if(i++==wanted)return String("/media/album/")+music::baseName(x);
  }
  return "";
}
int albumIndex=0;
uint32_t albumChangedAt=0;
void drawPhotoAlbum(){
  lcd.fillScreen(BG);
  int count=albumCount();
  if(count<=0){
    center(tr("Keine Fotos im Album","Brak zdjec w albumie","No photos in album"),160,92,2,GOLD);
    center(tr("Fotos ueber Web hochladen","Dodaj zdjecia przez web","Upload photos in web panel"),160,126,2,WHITE);
    return;
  }
  if(albumIndex>=count)albumIndex=0;
  String path=albumPathAt(albumIndex);
  lcd.setSwapBytes(true);
  if(path.length()&&TJpgDec.drawFsJpg(0,0,path,SD_MMC)==JDR_OK)return;
  center(tr("Foto kann nicht geladen werden","Nie mozna wczytac zdjecia","Photo cannot be loaded"),160,110,2,GOLD);
}

'''
s = s.replace('bool mountSD(){', album_helpers + 'bool mountSD(){')

s = s.replace(
    '  if(configScreen){if(all)drawConfig();}\n  else if(playerScreen){if(all)drawPlayer();}\n  else {if(all)drawClock();drawStatus(all);}',
    '  if(configScreen){if(all)drawConfig();}\n  else if(playerScreen){if(all)drawPlayer();}\n  else if(eventSettings.mode==countdown::PhotoAlbum){if(all)drawPhotoAlbum();}\n  else {if(all)drawClock();drawStatus(all);}'
)
s = s.replace(
    'if(!configScreen&&!playerScreen)key+=":"+currentArtwork()',
    'if(!configScreen&&!playerScreen)key+=":"+String(eventSettings.mode==countdown::PhotoAlbum?albumIndex:0)+":"+currentArtwork()'
)

album_api = r'''String albumListJson(){
  if(!sdReady)return "{\"error\":\"audio_no_card\"}";
  if(!SD_MMC.exists(albumDir()))SD_MMC.mkdir(albumDir());
  String out="{\"photos\":[";
  File dir=SD_MMC.open(albumDir());bool first=true;
  if(dir&&dir.isDirectory())for(File f=dir.openNextFile();f;f=dir.openNextFile()){
    if(f.isDirectory())continue;
    String n=f.name(),l=n;l.toLowerCase();
    if(!(l.endsWith(".jpg")||l.endsWith(".jpeg")))continue;
    if(!first)out+=",";
    first=false;
    out+="{\"name\":"+json(music::baseName(n))+",\"size\":"+String((unsigned long)f.size())+"}";
  }
  out+="]}";
  return out;
}
'''
s = s.replace('bool uploadAuthorized(){', album_api + 'bool uploadAuthorized(){')

s = s.replace(
    '    if(!safeMediaCategory(category)||(kind!="music"&&kind!="image")){mediaUploadError="invalid_fields";return;}\n    ensureMediaDirs(category);',
    '    if(kind=="album"){if(!sdReady){mediaUploadError="audio_no_card";return;}if(!SD_MMC.exists("/media"))SD_MMC.mkdir("/media");if(!SD_MMC.exists(albumDir()))SD_MMC.mkdir(albumDir());}\n    else {if(!safeMediaCategory(category)||(kind!="music"&&kind!="image")){mediaUploadError="invalid_fields";return;}ensureMediaDirs(category);}'
)
s = s.replace(
    '    if(kind=="image"){\n      if(!(lower.endsWith(".jpg")||lower.endsWith(".jpeg"))){mediaUploadError="image_jpeg_only";return;}\n      path=mediaImagePath(category);\n    }else{',
    '    if(kind=="album"){\n      if(!(lower.endsWith(".jpg")||lower.endsWith(".jpeg"))){mediaUploadError="image_jpeg_only";return;}\n      if(!filename.length()){mediaUploadError="invalid_fields";return;}\n      path=albumDir()+"/"+filename;\n    }else if(kind=="image"){\n      if(!(lower.endsWith(".jpg")||lower.endsWith(".jpeg"))){mediaUploadError="image_jpeg_only";return;}\n      path=mediaImagePath(category);\n    }else{'
)
s = s.replace(
    'String kind=server.arg("kind");size_t limit=kind=="image"?MAX_IMAGE_UPLOAD:MAX_MUSIC_UPLOAD;',
    'String kind=server.arg("kind");size_t limit=(kind=="image"||kind=="album")?MAX_IMAGE_UPLOAD:MAX_MUSIC_UPLOAD;'
)

routes = r'''  server.on("/api/album",HTTP_GET,[](){
    if(authorize())server.send(200,"application/json; charset=utf-8",albumListJson());
  });
  server.on("/api/album/delete",HTTP_POST,[](){
    if(!authorize(true))return;
    if(!sdReady){apiError(409,"audio_no_card");return;}
    String name=safeFileName(server.arg("name"));
    if(!name.length()){bad("invalid_fields");return;}
    String path=albumDir()+"/"+name;
    if(!SD_MMC.exists(path)){apiError(404,"not_found");return;}
    if(!SD_MMC.remove(path)){apiError(500,"delete_failed");return;}
    albumIndex=0;forceFull=true;dirty=true;ok();
  });
'''
s = s.replace('  server.on("/api/media",HTTP_GET,[](){', routes + '  server.on("/api/media",HTTP_GET,[](){')

s = s.replace(
    'server.handleClient();if(apActive)dns.processNextRequest();wifiTick();pollBoot();pollTouch();music::setCategory(musicCategory());music::loop(sdReady);',
    'server.handleClient();if(apActive)dns.processNextRequest();wifiTick();pollBoot();pollTouch();if(eventSettings.mode!=countdown::PhotoAlbum){music::setCategory(musicCategory());music::loop(sdReady);}'
)
s = s.replace(
    '  if(audioNotice&&uint32_t(millis()-audioNoticeAt)>=4000){audioNotice=false;dirty=true;}',
    '  if(eventSettings.mode==countdown::PhotoAlbum&&sdReady&&uint32_t(millis()-albumChangedAt)>=10000){int n=albumCount();if(n>0){albumIndex=(albumIndex+1)%n;albumChangedAt=millis();forceFull=true;dirty=true;}}\n  if(audioNotice&&uint32_t(millis()-audioNoticeAt)>=4000){audioNotice=false;dirty=true;}'
)

# Add album count to API status.
s = s.replace(
    '+",\\"vacationDate\\":"+json(vacation)+"}";',
    '+",\\"vacationDate\\":"+json(vacation)+",\\"albumCount\\":"+String(albumCount())+"}";'
)

main.write_text(s)

# ---- Web portal ----
s = web.read_text()
s = s.replace(
    '<option value="christmas" data-i18n="event_christmas"></option><option value="birthday"',
    '<option value="christmas" data-i18n="event_christmas"></option><option value="silvester" data-i18n="event_silvester"></option><option value="birthday"'
)
s = s.replace(
    '<option value="halloween" data-i18n="event_halloween"></option></select>',
    '<option value="halloween" data-i18n="event_halloween"></option><option value="nikolaus" data-i18n="event_nikolaus"></option><option value="photoalbum" data-i18n="event_photoalbum"></option></select>'
)

old_select = '''  <label for="mediaCategory" data-i18n="mediaOccasion"></label>
  <select id="mediaCategory">
    <option value="christmas">Weihnachten / Święta</option>
    <option value="silvester">Silvester / Sylwester</option>
    <option value="birthday">Geburtstag / Urodziny</option>
    <option value="halloween">Halloween</option>
    <option value="easter">Ostern / Wielkanoc</option>
    <option value="valentines">Valentinstag / Walentynki</option>
    <option value="vacation">Urlaub / Urlop</option>
    <option value="krampus">Krampus</option>
    <option value="nikolaus">Nikolaus / Mikołaj</option>
    <option value="grincz">Grincz</option>
    <option value="normal">Standard</option>
  </select>'''
new_select = '''  <p class="help"><span data-i18n="musicLinked"></span> <strong id="mediaOccasionLabel"></strong></p>
  <select id="mediaCategory" hidden>
    <option value="christmas">Weihnachten</option><option value="silvester">Silvester</option><option value="birthday">Geburtstag</option><option value="valentines">Valentinstag</option><option value="easter">Ostern</option><option value="vacation">Urlaub</option><option value="halloween">Halloween</option><option value="nikolaus">Nikolaus</option>
  </select>'''
s = s.replace(old_select,new_select)

s = re.sub(
    r'<div class="media-box"><h4 data-i18n="customImage"></h4>.*?</div>\n  </div>\n</section>',
    '''<div class="media-box"><h4 data-i18n="albumTitle"></h4><p class="help" data-i18n="albumHelp"></p>
      <div class="file-row"><input id="albumFiles" type="file" accept=".jpg,.jpeg,image/jpeg" multiple><button class="btn small" type="button" id="uploadAlbum" data-i18n="upload"></button></div>
      <div id="albumList" class="track-list"></div>
    </div>
  </div>
</section>''',
    s,count=1,flags=re.S
)

# i18n for countdown additions.
i18n = {
'de':('Silvester','Nikolaus','Fotoalbum','bis Silvester','bis Nikolaus'),
'pl':('Sylwester','Mikołaj','Fotoalbum','do Sylwestra','do Mikołaja'),
'en':('New Year','St. Nicholas','Photo album','until New Year','until St. Nicholas')
}
for lang,v in i18n.items():
    needle=f'"{lang}": {{"eventTitle"'
    repl=f'"{lang}": {{"event_silvester": "{v[0]}", "event_nikolaus": "{v[1]}", "event_photoalbum": "{v[2]}", "until_silvester": "{v[3]}", "until_nikolaus": "{v[4]}", "eventTitle"'
    s=s.replace(needle,repl)

media = {
'de':('Musik folgt automatisch dem gewählten Countdown:','Fotoalbum','Mehrere JPEG-Fotos hochladen. Im Modus Fotoalbum wechseln sie automatisch alle 10 Sekunden.'),
'pl':('Muzyka jest automatycznie przypisana do wybranego countdownu:','Fotoalbum','Dodaj wiele zdjęć JPEG. W trybie Fotoalbum będą zmieniane automatycznie co 10 sekund.'),
'en':('Music automatically follows the selected countdown:','Photo album','Upload multiple JPEG photos. In Photo album mode they rotate automatically every 10 seconds.')
}
for lang,v in media.items():
    needle=f'"{lang}":{{"mediaTitle"'
    repl=f'"{lang}":{{"musicLinked":"{v[0]}","albumTitle":"{v[1]}","albumHelp":"{v[2]}","mediaTitle"'
    s=s.replace(needle,repl)

s=s.replace(
    "$('eventDateHelp').textContent=['birthday','vacation'].includes(mode)?'':t('fixedHelp')",
    "$('eventDateHelp').textContent=['birthday','vacation','photoalbum'].includes(mode)?'':t('fixedHelp')"
)
s=s.replace(
    "$('headline').textContent=christmas?t('christmas'):mode==='christmas'?t('headline'):t('event_'+mode);",
    "$('headline').textContent=mode==='photoalbum'?t('event_photoalbum'):christmas?t('christmas'):mode==='christmas'?t('headline'):t('event_'+mode);"
)
s=s.replace(
    "$('days').textContent=ready?(christmas?'✧':s.days):'—';$('unit').textContent=christmas?t('celebrate'):t(s.days===1?'oneDay':'days');$('until').textContent=christmas?'24–26.12':t(mode==='christmas'?'until':'until_'+mode);",
    "$('days').textContent=mode==='photoalbum'?'▣':ready?(christmas?'✧':s.days):'—';$('unit').textContent=mode==='photoalbum'?t('event_photoalbum'):christmas?t('celebrate'):t(s.days===1?'oneDay':'days');$('until').textContent=mode==='photoalbum'?((s.albumCount||0)+' Fotos'):christmas?'24–26.12':t(mode==='christmas'?'until':'until_'+mode);"
)
s=s.replace(
    "$('heroNote').textContent=!s.timeValid?",
    "$('heroNote').textContent=mode==='photoalbum'?((s.albumCount||0)+' Fotos auf microSD'):!s.timeValid?"
)

s=s.replace(
    "if($('mediaCategory'))$('mediaCategory').value=s.musicCategory||'christmas';",
    "if($('mediaCategory')){$('mediaCategory').value=s.musicCategory||'christmas';$('mediaOccasionLabel').textContent=t('event_'+($('mediaCategory').value==='christmas'?'christmas':$('mediaCategory').value));}"
)
s=s.replace(
    "$('event').onchange=eventFields;",
    "$('event').onchange=()=>{eventFields();const m=$('event').value;if(m!=='photoalbum'&&$('mediaCategory')){$('mediaCategory').value=m;$('mediaOccasionLabel').textContent=t('event_'+m);mediaLoadedCategory='';loadMedia(true)}};"
)
s=s.replace("$('mediaCategory').onchange=()=>{mediaLoadedCategory='';loadMedia(true)};","")
s=s.replace("  $('imageState').textContent=t(r.customImage?'customImageActive':'factoryImageActive');\n  $('deleteImage').disabled=!r.customImage;\n","")
s=s.replace(
    "const category=$('mediaCategory').value,input=$(kind==='music'?'musicFile':'imageFile'),file=input.files[0];",
    "const category=$('mediaCategory').value,input=$('musicFile'),file=input.files[0];"
)
s=s.replace("$('uploadImage').onclick=()=>uploadMedia('image');\n$('deleteImage').onclick=async()=>{try{await api('/api/media/delete',{kind:'image',category:$('mediaCategory').value,name:''});mediaLoadedCategory='';await loadMedia(true);message('saved');await refresh()}catch(e){message(e.message,true)}};","")

album_js = r'''
async function loadAlbum(){
  if(!authenticated||!$('albumList'))return;
  try{
    const r=await api('/api/album');
    $('albumList').replaceChildren();
    if(!r.photos.length){const d=document.createElement('div');d.className='media-status';d.textContent='—';$('albumList').append(d);return}
    r.photos.forEach(f=>{
      const row=document.createElement('div');row.className='track';
      const name=document.createElement('span');name.className='track-name';name.textContent=f.name;
      const del=document.createElement('button');del.type='button';del.textContent='×';del.className='btn secondary small';
      del.onclick=async()=>{try{await api('/api/album/delete',{name:f.name});await loadAlbum();await refresh()}catch(e){message(e.message,true)}};
      row.append(name,del);$('albumList').append(row);
    });
  }catch(_){}
}
async function uploadAlbum(){
  const files=[...$('albumFiles').files];
  for(const file of files){
    const form=new FormData();form.append('file',file,file.name);
    const r=await fetch('/api/media/upload?kind=album&category=photoalbum',{method:'POST',headers:{'X-Peppi-CSRF':csrf},credentials:'same-origin',body:form});
    let body={};try{body=await r.json()}catch(_){}
    if(!r.ok)throw Error(body.error||'network_error');
  }
  $('albumFiles').value='';
  await loadAlbum();message('uploadOk');await refresh();
}
$('uploadAlbum').onclick=()=>uploadAlbum().catch(e=>message(e.message,true));
'''
pos=s.rfind('setInterval(')
if pos<0: pos=s.rfind('</script>')
s=s[:pos]+album_js+s[pos:]
s=s.replace('initialized=true}paint(s);','initialized=true;loadAlbum()}paint(s);')
s=s.replace('v4.3 ES3C28P','v4.8 ES3C28P')
web.write_text(s)
