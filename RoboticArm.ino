#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char* AP_SSID = "ArmBot-ESP32";
const char* AP_PASSWORD = "ArmControl123";

const uint8_t SERVO_PINS[4] = {13, 14, 25, 26};
const uint8_t IR_SENSOR_PIN = 27;
const bool IR_ACTIVE_LOW = true;
const int SERVO_MIN_US = 500;
const int SERVO_MAX_US = 2400;

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ArmBot Control</title>
<style>
:root{color-scheme:dark;--bg:#0b1020;--panel:#151e32;--line:#283650;--text:#eef3ff;--muted:#9baac2;--accent:#7c9cff;--good:#39d6a0;--warn:#ffbd69}
*{box-sizing:border-box}body{margin:0;background:radial-gradient(circle at top,#172747,var(--bg) 55%);font-family:Arial,sans-serif;color:var(--text);min-height:100vh}
main{width:min(980px,94%);margin:28px auto}.top{display:flex;justify-content:space-between;align-items:center;gap:16px;flex-wrap:wrap;margin-bottom:22px}
h1{font-size:clamp(25px,5vw,36px);margin:0}.sub{color:var(--muted);margin-top:7px;font-size:14px}
.pill,.card{background:var(--panel);border:1px solid var(--line);border-radius:16px}.pill{padding:10px 14px;color:var(--good);font-size:13px}
.grid{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:16px}
.card{padding:20px}.wide{grid-column:1/-1}.card h2{font-size:17px;margin:0 0 17px}
.sensor{display:flex;align-items:center;justify-content:space-between;gap:12px;padding:13px 0;border-top:1px solid var(--line)}
.sensor:first-of-type{border-top:0}.value{font-weight:700}.status{color:var(--good)}
.joint{margin:18px 0}.joint-head{display:flex;justify-content:space-between;margin-bottom:10px;font-size:14px}
input[type=range]{width:100%;accent-color:var(--accent)}
button{border:0;border-radius:10px;background:var(--accent);color:#071126;padding:12px 16px;font-weight:700;cursor:pointer}
button.secondary{background:#263650;color:var(--text)}button:active{transform:scale(.98)}
.actions{display:flex;gap:10px;flex-wrap:wrap}.note{color:var(--muted);font-size:12px;line-height:1.5;margin-top:16px}
@media(max-width:650px){.grid{grid-template-columns:1fr}.wide{grid-column:auto}.card{padding:16px}}
</style>
</head>
<body>
<main>
<header class="top"><div><h1>ArmBot Control</h1><div class="sub">Wi-Fi controlled 4-DOF robotic arm</div></div><div class="pill">● ESP32 ACCESS POINT</div></header>
<section class="grid">
<div class="card">
<h2>Joint Controls</h2>
<div class="joint"><div class="joint-head"><label for="j0">Base rotation</label><strong id="v0">90°</strong></div><input id="j0" type="range" min="0" max="180" value="90"></div>
<div class="joint"><div class="joint-head"><label for="j1">Shoulder</label><strong id="v1">90°</strong></div><input id="j1" type="range" min="0" max="180" value="90"></div>
<div class="joint"><div class="joint-head"><label for="j2">Elbow</label><strong id="v2">90°</strong></div><input id="j2" type="range" min="0" max="180" value="90"></div>
<div class="joint"><div class="joint-head"><label for="j3">Gripper</label><strong id="v3">90°</strong></div><input id="j3" type="range" min="0" max="180" value="90"></div>
<div class="actions"><button id="home">Home position</button><button class="secondary" id="refresh">Refresh status</button></div>
</div>
<div class="card">
<h2>Object Detection</h2>
<div class="sensor"><span>IR obstacle sensor</span><strong class="value" id="object">Checking...</strong></div>
<div class="sensor"><span>Detection input</span><strong class="value" id="ir">—</strong></div>
<div class="sensor"><span>Connection</span><strong class="status" id="connection">Connected</strong></div>
<p class="note">Detection is provided by the connected IR obstacle sensor. It indicates whether an object is detected within the sensor's effective range; it is not camera-based recognition or distance measurement.</p>
</div>
<div class="card wide">
<h2>System Information</h2>
<div class="sensor"><span>Wi-Fi network</span><strong class="value">ArmBot-ESP32</strong></div>
<div class="sensor"><span>Dashboard address</span><strong class="value">http://192.168.4.1</strong></div>
<div class="sensor"><span>Control</span><strong class="value">Manual joint positioning</strong></div>
</div>
</section>
</main>
<script>
const sliders=[0,1,2,3].map(i=>document.getElementById('j'+i));
let timers=[];
function setJoint(i,value){
 document.getElementById('v'+i).textContent=value+'°';
 clearTimeout(timers[i]);
 timers[i]=setTimeout(async()=>{
  try{const r=await fetch(`/api/servo?joint=${i}&angle=${value}`);if(!r.ok)throw Error();}
  catch(e){document.getElementById('connection').textContent='Connection lost';}
 },100);
}
sliders.forEach((s,i)=>s.addEventListener('input',()=>setJoint(i,s.value)));
async function refreshState(){
 try{
  const r=await fetch('/api/state');if(!r.ok)throw Error();
  const d=await r.json();
  d.angles.forEach((a,i)=>{sliders[i].value=a;document.getElementById('v'+i).textContent=a+'°';});
  document.getElementById('object').textContent=d.objectDetected?'Object detected':'No object detected';
  document.getElementById('object').style.color=d.objectDetected?'var(--warn)':'var(--good)';
  document.getElementById('ir').textContent=d.irLevel;
  document.getElementById('connection').textContent='Connected';
 }catch(e){document.getElementById('connection').textContent='Connection lost';}
}
document.getElementById('home').addEventListener('click',async()=>{
 await Promise.all([0,1,2,3].map(i=>fetch(`/api/servo?joint=${i}&angle=90`)));
 refreshState();
});
document.getElementById('refresh').addEventListener('click',refreshState);
refreshState();setInterval(refreshState,1200);
</script>
</body>
</html>
)rawliteral";

WebServer server(80);
Servo servos[4];
int angles[4] = {90, 90, 90, 90};

bool objectDetected() {
  int level = digitalRead(IR_SENSOR_PIN);
  return IR_ACTIVE_LOW ? level == LOW : level == HIGH;
}

void sendState() {
  String json = "{\"angles\":[";
  for (int i = 0; i < 4; i++) {
    if (i) json += ",";
    json += String(angles[i]);
  }
  json += "],\"objectDetected\":";
  json += objectDetected() ? "true" : "false";
  json += ",\"irLevel\":";
  json += String(digitalRead(IR_SENSOR_PIN));
  json += "}";
  server.send(200, "application/json", json);
}

void handleServo() {
  if (!server.hasArg("joint") || !server.hasArg("angle")) {
    server.send(400, "text/plain", "Missing joint or angle");
    return;
  }

  int joint = server.arg("joint").toInt();
  int angle = server.arg("angle").toInt();

  if (joint < 0 || joint > 3 || angle < 0 || angle > 180) {
    server.send(400, "text/plain", "Invalid joint or angle");
    return;
  }

  angles[joint] = angle;
  servos[joint].write(angle);
  server.send(200, "application/json", "{\"ok\":true}");
}

void setup() {
  Serial.begin(115200);
  pinMode(IR_SENSOR_PIN, INPUT);

  for (int i = 0; i < 4; i++) {
    servos[i].setPeriodHertz(50);
    servos[i].attach(SERVO_PINS[i], SERVO_MIN_US, SERVO_MAX_US);
    servos[i].write(angles[i]);
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", INDEX_HTML);
  });
  server.on("/api/state", HTTP_GET, sendState);
  server.on("/api/servo", HTTP_GET, handleServo);
  server.begin();

  Serial.println();
  Serial.print("Wi-Fi network: ");
  Serial.println(AP_SSID);
  Serial.print("Dashboard: http://");
  Serial.println(WiFi.softAPIP());
}

void loop() {
  server.handleClient();
}
