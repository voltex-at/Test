#!/usr/bin/env python3
import hashlib, json, re, sys
from pathlib import Path

root=Path(sys.argv[1]).resolve()
edits_path=Path(sys.argv[2]).resolve()
spec=json.loads(edits_path.read_text(encoding="utf-8"))

def sha(s): return hashlib.sha256(s.encode("utf-8")).hexdigest()

for rel, item in spec.items():
    p=root/rel
    s=p.read_text(encoding="utf-8")
    if sha(s)!=item["old_sha256"]:
        raise SystemExit(f"OLD HASH MISMATCH: {rel} {sha(s)}")
    for start,end,repl in sorted(item["edits"], key=lambda x:x[0], reverse=True):
        s=s[:start]+repl+s[end:]
    if sha(s)!=item["new_sha256"]:
        raise SystemExit(f"NEW HASH MISMATCH: {rel} {sha(s)}")
    p.write_text(s,encoding="utf-8")
    print("patched",rel)

# Keep exactly one factory main/Wi-Fi/MP3 image per Countdown Mode,
# plus the 3 Christmas-only special scenes. "normal" becomes Christmas.
art=root/"include/Artwork.h"
src=art.read_text(encoding="utf-8")
mapping=[
 ("birthday","birthday"),
 ("normal","christmas"),
 ("easter","easter"),
 ("grincz","grincz"),
 ("halloween","halloween"),
 ("krampus","krampus"),
 ("mp3_birthday","mp3_birthday"),
 ("mp3_christmas","mp3_christmas"),
 ("mp3_easter","mp3_easter"),
 ("mp3_halloween","mp3_halloween"),
 ("mp3_silvester","mp3_silvester"),
 ("mp3_vacation","mp3_vacation"),
 ("nikolaus","nikolaus"),
 ("silvester","silvester"),
 ("vacation","vacation"),
 ("wifi_birthday","wifi_birthday"),
 ("wifi","wifi_christmas"),
 ("wifi_easter","wifi_easter"),
 ("wifi_halloween","wifi_halloween"),
 ("wifi_silvester","wifi_silvester"),
 ("wifi_vacation","wifi_vacation"),
]
blocks=[]
entries=[]
for source,dest in mapping:
    pat=rf"const uint8_t jpg_{re.escape(source)}\[\] PROGMEM = \{{.*?\n\}};"
    m=re.search(pat,src,re.S)
    if not m: raise SystemExit(f"missing artwork: {source}")
    block=m.group(0)
    if source!=dest:
        block=block.replace(f"jpg_{source}",f"jpg_{dest}",1)
    blocks.append(block)
    entries.append(f'{{"{dest}",jpg_{dest},sizeof(jpg_{dest})}},')
out="#pragma once\n#include <Arduino.h>\nnamespace artwork {\nstruct Image {const char* name; const uint8_t* data; size_t size;};\n"
out+="\n".join(blocks)+"\nconst Image images[] = {\n"+"\n".join(entries)+"\n};\n"
out+='inline const Image& get(const String& name){for(const auto& im:images)if(name==im.name)return im;return images[0];}\n}\n'
art.write_text(out,encoding="utf-8")
print("artwork images",len(mapping),"bytes",art.stat().st_size)

# Re-embed the patched web UI into flash.
html=(root/"web/index.html").read_text(encoding="utf-8")
if ')PEPPI"' in html: raise SystemExit("bad raw-string terminator in web UI")
(root/"include/WebPage.h").write_text(
    '#pragma once\n#include <Arduino.h>\nconst char WEB_PAGE[] PROGMEM = R"PEPPI('+html+')PEPPI";\n',
    encoding="utf-8")
print("web bytes",len(html.encode("utf-8")))
