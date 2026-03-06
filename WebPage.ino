// neu optimiert
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>ESP32 Remote</title>
<style>

body{
    margin:0;
    font-family:Arial;
    background:#111;
    color:white;
}

.section{
    height:100vh;
    display:flex;
    flex-direction:column;
    justify-content:center;
    align-items:center;
}

h1,h2,h3{
    margin:10px;
}

/* ===== Standard große Buttons ===== */
.bigBtn{
    width:85%;
    max-width:400px;
    height:75px;
    font-size:24px;
    margin:12px;
    border:none;
    border-radius:16px;
    color:white;
}

/* ===== Farben ===== */
.green{background:#1faa00;}
.red{background:#cc0000;}
.gray{background:#555;}
.gray:hover{background:#777;}

/* ===== Remote Bereich ===== */
.remote{
    text-align:center;
    padding-bottom:40px;
}

.remote button{
    width:90px;
    height:90px;
    font-size:26px;
    margin:8px;
    border:none;
    border-radius:18px;
    color:white;
    transition: all 0.1s ease;
}

/* Standardfarbe NUR wenn keine Farbklasse gesetzt */
.remote button:not(.green):not(.red):not(.gray){
    background:#333;
}

/* gedrückt Effekt */
.remote button:active{
    transform: scale(0.92);
    filter: brightness(1.4);
}

/* Visueller Feedback Flash */
.btnFlash{
    animation: flash 0.25s ease;
}

@keyframes flash{
    0%   { filter: brightness(1); }
    50%  { filter: brightness(2); }
    100% { filter: brightness(1); }
}

.power{
    width:220px !important;
    height:80px !important;
    font-size:26px;
}

.ok{
    background:#1faa00 !important;
}

/* ===== Status ===== */
.statusBox{
    font-size:20px;
    margin:15px;
}

/* ===== Log ===== */
#log{
    width:90%;
    max-width:400px;
    height:200px;
    background:#000;
    color:#00ff00;
    padding:10px;
    overflow:auto;
    font-size:14px;
    border-radius:10px;
}

.dpad{
    position:relative;   /* wichtig */
    display:flex;
    flex-direction:column;
    align-items:center;
    margin:20px 0;
}


.middleRow{
    display:flex;
    align-items:center;
    justify-content:center;
}

.nav{
    width:120px;
    height:70px;
    background:#333;
    font-size:20px;
}

/* Back relativ zur Mitte positionieren */
.bnav{
    position:absolute;
    top:0;
    left:50%;                       /* Mitte */
    transform: translateX(-240px);  /* Abstand von der Mitte */
    width:120px;
    height:70px;
    background:#333;
    font-size:20px;
}

.ok{
    width:100px;
    height:100px;
    background:#1faa00 !important;
    font-size:22px;
    margin:10px;
}

.row{
    display:flex;
    justify-content:center;
    flex-wrap:wrap;
}

.dpad .nav {
    font-weight: bold;
    font-size: 2.2em;
}

</style>
</head>
<body>

<!-- ===== Seite 1: TV ===== -->
<h2>FireTV Remote</h2>

<div class="remote">

  <button id="powerBtn" class="power red" onclick="toggleTV()">
    TV ist AUS
  </button>

<!-- D-Pad -->
<div class="dpad">
    <!-- Back außerhalb des Kreuzes -->
    <button class="bnav" onclick="sendCmd(31,this)">Back</button>
    <button class="nav up" onclick="sendCmd(20,this)">&#9650;</button>
    <div class="middleRow">
        <button class="nav left" onclick="sendCmd(22,this)">&#9668;</button>
        <button class="ok" onclick="sendCmd(37,this)">OK</button>
        <button class="nav right" onclick="sendCmd(23,this)">&#9658;</button>
    </div>
    <button class="nav down" onclick="sendCmd(21,this)">&#9660;</button>
</div>

  <!-- Lautstärke -->
  <div class="row">
      <button onclick="sendCmd(16,this)">V +</button>
      <button onclick="sendCmd(17,this)">V -</button>
      <button onclick="sendCmd(15,this)">Mute</button>
  </div>

  <!-- Media -->
  <div class="row">
      <button onclick="sendCmd(50,this)"> << </button>
      <button onclick="sendCmd(53,this)">PLAY</button>
      <button onclick="sendCmd(52,this)"> >> </button>
  </div>

  <div class="row">
      <button onclick="sendCmd(54,this)">STOP</button>
  </div>

</div>

<!-- ===== Seite 2: Texteingabe ===== -->
<div class="section">

<h2>Text senden</h2>

<input type="text" id="textInput" placeholder="Text eingeben..."
       style="width:85%;max-width:400px;height:60px;
              font-size:22px;border-radius:12px;
              border:none;padding:10px;">

<button class="bigBtn green" onclick="sendText()">
Text senden
</button>

</div>

<!-- ===== Seite 2: Bluetooth ===== -->
<div class="section">

<h2>Bluetooth</h2>

<div class="statusBox">
Connected: <span id="conn">-</span><br>
Advertising: <span id="adv">-</span><br>
</div>

<button id="startBtn" onclick="send('/start')" class="bigBtn green">
Start Advertising
</button>

<button id="stopBtn" onclick="send('/stop')" class="bigBtn gray">
Stop Advertising
</button>

<button onclick="send('/clients')" class="bigBtn gray">
Print Connected
</button>

<button onclick="send('/bonds')" class="bigBtn gray">
Print Bonds
</button>

<button onclick="send('/disconnect')" class="bigBtn gray">
Disconnect
</button>

<button onclick="send('/deletebonds')" class="bigBtn red">
Delete Bonds
</button>

<h3>BLE Log</h3>
<div id="log"></div>

</div>

<script>

function toggleTV(){
    fetch('/cmd?c=61')
    .then(r=>r.text())
    .then(()=>{
        loadStatus();
    });
}

function updateTV(on){
    let btn = document.getElementById("powerBtn");

    btn.classList.add("btnFlash");
    setTimeout(()=>btn.classList.remove("btnFlash"),250);

    if(on){
        btn.innerText = "TV ist EIN";
        btn.className = "power green";
    }else{
        btn.innerText = "TV ist AUS";
        btn.className = "power red";
    }
}

function send(url){
    fetch(url).then(()=>loadStatus());
}

function loadStatus(){
    fetch('/status')
    .then(r=>r.json())
    .then(j=>{

        // Connected Anzeige
        let conn = document.getElementById("conn");
        conn.innerText = j.connected ? "JA" : "NEIN";
        conn.style.color = j.connected ? "#1faa00" : "#cc0000";

        // Advertising Anzeige
        let adv = document.getElementById("adv");
        adv.innerText = j.advertising ? "ON" : "OFF";
        adv.style.color = j.advertising ? "#1faa00" : "#cc0000";

        // Start/Stop Button einfärben
        let startBtn = document.getElementById("startBtn");
        let stopBtn = document.getElementById("stopBtn");

        if(j.advertising){
            startBtn.className = "bigBtn " + (j.advertising ? "gray" : "green");
            stopBtn.className  = "bigBtn " + (j.advertising ? "green" : "gray"); 
            } else {
            startBtn.className = "bigBtn " + (j.advertising ? "gray" : "green");
            stopBtn.className  = "bigBtn " + (j.advertising ? "green" : "gray"); 
        }

        // TV Button
        updateTV(j.tvOn);
    });
}

function loadLog(){
    fetch('/log')
    .then(r=>r.text())
    .then(t=>{
        document.getElementById("log").innerText = t;
    });
}

function sendText(){
    let txt = document.getElementById("textInput").value;

    if(txt.length === 0) return;

    fetch("/text?msg=" + encodeURIComponent(txt))
    .then(()=> {
        document.getElementById("textInput").value="";
    });
}

// Enter-Taste im Feld erlaubt direktes Senden
document.addEventListener("DOMContentLoaded", function(){
    document.getElementById("textInput")
    .addEventListener("keypress", function(e){
        if(e.key === "Enter"){
            sendText();
        }
    });
});

function sendCmd(cmd, el){

    // Visueller Sofort-Effekt
    if(el){
        el.classList.add("btnFlash");
        setTimeout(()=>{
            el.classList.remove("btnFlash");
        },250);
    }

    fetch('/cmd?c=' + cmd);
}

setInterval(loadStatus,1000);
setInterval(loadLog,2000);

</script>

</body>
</html>
)rawliteral";

void setupWeb() {
    server.on("/", []() {
        server.send(200, "text/html", index_html);
    });

    server.on("/start", []() {
        startAdvertisingWindow();
        server.send(200, "text/plain", "Advertising 30s gestartet");
    });

        server.on("/stop", []() {
        keyboardBLE_stopAdvertising_HAL();
        server.send(200, "text/plain", "Advertising stopped");
    });

    server.on("/clients", []() {
        keyboardBLE_printConnectedClients_HAL();
        server.send(200, "text/plain", "Printed to serial");
    });

    server.on("/disconnect", []() {
        keyboardBLE_disconnectAllClients_HAL();
        server.send(200, "text/plain", "Disconnected");
    });

    server.on("/bonds", []() {
        keyboardBLE_printBonds_HAL();
        server.send(200, "text/plain", "Printed bonds to serial");
    });

    server.on("/deletebonds", []() {
        keyboardBLE_deleteBonds_HAL();
        delay(100); 
        keyboardBLE_printBonds_HAL();
        delay(500); 
        ESP.restart();
    server.send(200, "text/plain", "Bonds deleted - rebooting");
    });

    server.on("/status", []() {
        String json = "{";
        json += "\"connected\":" + String(bleConnected ? "true" : "false") + ",";
        json += "\"advertising\":" + String(bleAdvertising ? "true" : "false") + ",";
        json += "\"tvOn\":" + String(tvOn ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    });

    server.on("/log", []() {
        server.send(200, "text/plain", bleLog);
    });

    server.on("/toggleTV", []() {
        tvOn = !tvOn;
        digitalWrite(RELAY_PIN, tvOn ? HIGH : LOW);
        server.send(200, "text/plain", tvOn ? "ON" : "OFF");
    });

    server.on("/cmd", []() {
        if (!server.hasArg("c")) {
            server.send(400, "text/plain", "missing command");
            return;
        }

        uint16_t cmd = server.arg("c").toInt();

        // POWER geht immer
        if (cmd == 61) {
            ActRelais(61);
            server.send(200, "text/plain", "OK");
            return;
        }

        if (!keyboardBLE_isConnected_HAL()) {
            server.send(200, "text/plain", "not connected");
            return;
        }

        switch (cmd)
        {
            case 16: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_VOLUME_UP); break;
            case 17: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_VOLUME_DOWN); break;
            case 22: if (checkCommandRate(cmd,100)) keyboardBLE_write_HAL(KEY_LEFT_ARROW); break;
            case 23: if (checkCommandRate(cmd,100)) keyboardBLE_write_HAL(KEY_RIGHT_ARROW); break;
            case 20: if (checkCommandRate(cmd,100)) keyboardBLE_write_HAL(KEY_UP_ARROW); break;
            case 21: if (checkCommandRate(cmd,100)) keyboardBLE_write_HAL(KEY_DOWN_ARROW); break;
            case 37: if (checkCommandRate(cmd,100)) keyboardBLE_write_HAL(KEY_RETURN); break;
            case 31: if (checkCommandRate(cmd,100)) keyboardBLE_write_HAL(KEY_BACKSPACE); break;
            case 53:
            case 48: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_PLAY_PAUSE); break;
            case 15: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_MUTE); break;
            case 54: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_STOP); break;
            case 52: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_FASTFORWARD); break;
            case 50: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_REWIND); break;
            case 59: if (checkCommandRate(cmd,100)) consumerControlBLE_write_HAL(KEY_MEDIA_WWW_HOME); break;
        }

        server.send(200, "text/plain", "OK");
    });
    
    server.on("/text", [](){

    if(!server.hasArg("msg")){
        server.send(400,"text/plain","no text");
        return;
    }

    if(!keyboardBLE_isConnected_HAL()){
        server.send(200,"text/plain","not connected");
        return;
    }

    String text = server.arg("msg");

    for(size_t i=0; i<text.length(); i++){
        keyboardBLE_write_HAL(text[i]);
        delay(500);   // Pause für Stabilität und zum Senden des Textes
        keyboardBLE_write_HAL(KEY_RETURN);
    }

    server.send(200,"text/plain","OK");
    });

    server.begin();
}
/*  
Einfache Pfeile (schmal)
Nach oben: ↑ (&#8593; oder &uarr;)
Nach unten: ↓ (&#8595; oder &darr;)
Nach links: ← (&#8592; oder &larr;)
Nach rechts: → (&#8594; oder &rarr;)
Fette/dicke Pfeile
Nach oben: ⇑ (&#8657; oder &uArr;)
Nach unten: ⇓ (&#8659; oder &dArr;)
Nach links: ⇐(&#8656;oder&lArr;`)
Nach rechts: ⇒ (&#8658; oder &rArr;)
Schwarze Dreiecke
Nach oben: ▲ (&#9650; oder &blacktriangle;)
Nach unten: ▼ (&#9660; oder &blacktriangledown;)
Nach links: ◄ (&#9668; oder &blacktriangleleft;)
Nach rechts: ► (&#9658; oder &blacktriangleright;)
Weiße Dreiecke
Nach oben: △ (&#9651; oder &bigtriangleup;)
Nach unten: ▽ (&#9661; oder &bigtriangledown;)
Nach links: ◁ (&#9669; oder &triangleleft;)
Nach rechts: ▷ (&#9659; oder &triangleright;)
Diagonale Pfeile
Oben-links: ↖ (&#8598; oder &nwarr;)
Oben-rechts: ↗ (&#8599; oder &nearr;)
Unten-links: ↙ (&#8600; oder &swarr;)
Unten-rechts: ↘ (&#8601; oder &searr;)
Zusätzliche diagonale Pfeile (fett)
Nord-Ost: ⇗ (&#8663; oder &neArr;)
Süd-Ost: ⇘ (&#8664; oder &seArr;)
Süd-West: ⇙ (&#8665; oder &swArr;)
Nord-West: ⇖ (&#8662; oder &nwArr;)
Beispiel für deine D-Pad-Steuerung mit numerischen Entitäten
*/