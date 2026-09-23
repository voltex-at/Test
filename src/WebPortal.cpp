#include "WebPortal.h"
#include "Config.h"
#include <WiFi.h>

WebPortal::WebPortal(SettingsManager& settings, WifiManager& wifi)
  : settings_(settings), wifi_(wifi) {}

void WebPortal::begin() {
  server_.on("/", HTTP_GET, [this]() { handleRoot(); });
  server_.on("/save", HTTP_POST, [this]() { handleSave(); });
  server_.on("/reset-wifi", HTTP_POST, [this]() { handleResetWifi(); });

  // Typowe endpointy wykorzystywane przez telefony do wykrywania captive portal.
  server_.on("/generate_204", HTTP_ANY, [this]() { handleRoot(); });
  server_.on("/hotspot-detect.html", HTTP_ANY, [this]() { handleRoot(); });
  server_.on("/fwlink", HTTP_ANY, [this]() { handleRoot(); });
  server_.onNotFound([this]() { handleNotFound(); });
  server_.begin();
}

void WebPortal::handle() {
  server_.handleClient();
}

String WebPortal::htmlEscape(const String& value) {
  String s = value;
  s.replace("&", "&amp;");
  s.replace("<", "&lt;");
  s.replace(">", "&gt;");
  s.replace("\"", "&quot;");
  s.replace("'", "&#39;");
  return s;
}

bool WebPortal::parseBirthday(const String& value, uint8_t& day, uint8_t& month) {
  day = 0; month = 0;
  if (value.length() < 4 || value.length() > 5) return false;
  int sep = value.indexOf('.');
  if (sep < 0) sep = value.indexOf('-');
  if (sep < 1) return false;
  int d = value.substring(0, sep).toInt();
  int m = value.substring(sep + 1).toInt();
  if (d < 1 || d > 31 || m < 1 || m > 12) return false;
  static const uint8_t mdays[] = {31,29,31,30,31,30,31,31,30,31,30,31};
  if (d > mdays[m - 1]) return false;
  day = static_cast<uint8_t>(d);
  month = static_cast<uint8_t>(m);
  return true;
}

String WebPortal::pageHtml(const DeviceSettings& s) {
  String birthday = "";
  if (s.birthdayDay && s.birthdayMonth) {
    char b[6];
    snprintf(b, sizeof(b), "%02u.%02u", s.birthdayDay, s.birthdayMonth);
    birthday = b;
  }

  String status;
  if (wifi_.connected()) {
    status = String("Verbunden mit <b>") + htmlEscape(WiFi.SSID()) + "</b> · IP " + wifi_.ip();
  } else if (wifi_.apMode()) {
    status = String("Setup-Modus · WLAN <b>") + AppConfig::AP_SSID + "</b> · " + wifi_.ip();
  } else {
    status = "Nicht verbunden";
  }

  String html;
  html.reserve(7200);
  html += R"HTML(<!doctype html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>Weihnachtsuhr</title><style>
:root{color-scheme:dark;--bg:#061638;--card:#0d2859dd;--gold:#ffd56b;--red:#c82a3a;--green:#1f8a5b;--text:#f6f8ff;--muted:#b8c7e6}*{box-sizing:border-box}body{margin:0;min-height:100vh;font-family:system-ui,-apple-system,Segoe UI,sans-serif;color:var(--text);background:radial-gradient(circle at 80% 8%,#244a8b 0 5%,transparent 20%),linear-gradient(#061638,#0a2a5c 62%,#dbeaff 63%,#edf5ff);overflow-x:hidden}.snow{position:fixed;inset:0;pointer-events:none;background-image:radial-gradient(circle,#fff 0 2px,transparent 2.5px),radial-gradient(circle,#fff 0 1.5px,transparent 2px);background-size:64px 64px,91px 91px;background-position:0 0,20px 30px;opacity:.55;animation:fall 10s linear infinite}@keyframes fall{to{background-position:0 64px,20px 121px}}main{position:relative;max-width:720px;margin:auto;padding:28px 16px 50px}.hero{text-align:center;margin:8px 0 22px}.hero h1{font-size:clamp(30px,8vw,52px);margin:0;color:var(--gold);text-shadow:0 3px 0 #7b1423}.hero p{color:var(--muted);margin:7px 0}.card{background:var(--card);border:1px solid #ffffff29;border-radius:22px;padding:20px;box-shadow:0 18px 45px #00102c66;backdrop-filter:blur(8px)}.status{background:#061b3f;border-radius:14px;padding:12px 14px;margin-bottom:18px;color:#dce8ff}.grid{display:grid;grid-template-columns:1fr 1fr;gap:14px}@media(max-width:560px){.grid{grid-template-columns:1fr}}label{display:block;font-weight:700;margin:4px 0 7px}input{width:100%;font-size:16px;padding:13px 14px;border-radius:12px;border:1px solid #ffffff35;background:#061b3f;color:#fff;outline:none}input:focus{border-color:var(--gold);box-shadow:0 0 0 3px #ffd56b22}.section{margin-top:20px}.section h2{font-size:19px;margin:0 0 13px;color:var(--gold)}button{border:0;border-radius:13px;padding:13px 18px;font-size:16px;font-weight:800;cursor:pointer}.save{width:100%;margin-top:22px;background:linear-gradient(135deg,#db3347,#9c1730);color:white}.danger{margin-top:12px;background:#ffffff12;color:#ffd7dd;border:1px solid #ffffff25}.note{font-size:13px;color:var(--muted);line-height:1.45;margin-top:10px}.tree{font-size:34px;display:inline-block}small{color:var(--muted)}</style></head><body><div class="snow"></div><main><div class="hero"><div class="tree">🎄 ✨ 🎄</div><h1>Weihnachtsuhr</h1><p>Konfiguration deiner Weihnachtsuhr</p></div><div class="card"><div class="status">)HTML";
  html += status;
  html += R"HTML(</div><form method="post" action="/save"><div class="section"><h2>📶 WLAN</h2><div class="grid"><div><label for="ssid">Netzwerkname</label><input id="ssid" name="ssid" autocomplete="off" required value=")HTML";
  html += htmlEscape(s.ssid);
  html += "\"";
  html += R"HTML(></div><div><label for="pass">Passwort</label><input id="pass" name="pass" type="password" autocomplete="new-password" placeholder="Nur bei Änderung eingeben"></div></div><p class="note">Wenn das Passwort leer bleibt und der Netzwerkname nicht geändert wurde, bleibt das gespeicherte Passwort erhalten.</p></div><div class="section"><h2>🎂 Geburtstag</h2><div class="grid"><div><label for="bname">Name</label><input id="bname" name="bname" maxlength="24" value=")HTML";
  html += htmlEscape(s.birthdayName);
  html += "\"";
  html += R"HTML( placeholder="z. B. Anna"></div><div><label for="birthday">Datum (TT.MM)</label><input id="birthday" name="birthday" inputmode="numeric" maxlength="5" pattern="[0-9]{1,2}[.\-][0-9]{1,2}" value=")HTML";
  html += birthday;
  html += "\"";
  html += R"HTML( placeholder="14.03"></div></div></div><button class="save" type="submit">💾 SPEICHERN & NEUSTARTEN</button></form><form method="post" action="/reset-wifi"><button class="danger" type="submit">WLAN-Daten löschen</button></form><p class="note">Nach dem Speichern startet die Uhr neu, verbindet sich mit dem WLAN und synchronisiert die Uhrzeit automatisch.</p></div></main></body></html>)HTML";
  return html;
}

void WebPortal::handleRoot() {
  server_.sendHeader("Cache-Control", "no-store");
  server_.send(200, "text/html; charset=utf-8", pageHtml(settings_.load()));
}

void WebPortal::handleSave() {
  DeviceSettings old = settings_.load();
  String ssid = server_.arg("ssid");
  String pass = server_.arg("pass");
  String name = server_.arg("bname");
  String birthday = server_.arg("birthday");
  ssid.trim(); name.trim(); birthday.trim();

  if (ssid.isEmpty()) {
    server_.send(400, "text/plain; charset=utf-8", "WLAN-Name darf nicht leer sein.");
    return;
  }
  if (pass.isEmpty() && ssid == old.ssid) pass = old.password;

  uint8_t day = 0, month = 0;
  if (!birthday.isEmpty() && !parseBirthday(birthday, day, month)) {
    server_.send(400, "text/plain; charset=utf-8", "Geburtsdatum bitte als TT.MM eingeben.");
    return;
  }

  settings_.saveWifi(ssid, pass);
  settings_.saveBirthday(name, day, month);
  server_.send(200, "text/html; charset=utf-8", "<meta charset='utf-8'><style>body{font-family:system-ui;background:#061638;color:white;text-align:center;padding:50px}h1{color:#ffd56b}</style><h1>🎄 Gespeichert!</h1><p>Die Weihnachtsuhr startet neu…</p>");
  restartRequested_ = true;
}

void WebPortal::handleResetWifi() {
  settings_.clearWifi();
  server_.send(200, "text/html; charset=utf-8", "<meta charset='utf-8'><style>body{font-family:system-ui;background:#061638;color:white;text-align:center;padding:50px}h1{color:#ffd56b}</style><h1>WLAN gelöscht</h1><p>Neustart in den Setup-Modus…</p>");
  restartRequested_ = true;
}

void WebPortal::handleNotFound() {
  // Captive portal: wszystko wraca na strone glowna.
  server_.sendHeader("Location", "/", true);
  server_.send(302, "text/plain", "");
}
