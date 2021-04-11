#ifdef ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <ESPmDNS.h>
#else
#error "Board not found"
#endif

#include <WebSocketsServer.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
WebSocketsServer websockets(81);

int dl = D1;
int cl1 = D3;
int cl2 = D5;
int tl = D7;

char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
    <style>
        <style>
        body {
            font-family: arial;
            background: radial-gradient(#c2d2f8, #f5f5f5, #c2d2f8);
        }
        button {
            padding:10px;
            background-color: #83ec7f;
            color: #111111;
            cursor:pointer;
            border-radius: 10px;
        }
        .cont1 {
            width: 46.5%;
            border-radius: 15px;
            float: left;
            margin: 30px;
        }
        .cont2 {
            background: #0f0f0f;
            text-align: center;
            color: #ffffff;
            font-size: 25px;
            margin: 10px;
            padding: 5px;
            border-radius: 25px;
            text-decoration: none;
        }
        .cont3 {
            border-color: #0f0f0f;
            text-align: center;
            margin: 10px;
            padding: 5px;
            border-radius: 25px;
            text-decoration: none;
        }
        #a1 {font-size: 55px;display: inline-flex;}
        #a2 {font-size: 30px;}
        #a3 {font-size: 40px;}
        #output {
            /*background-color:#F9F9F9;*/
            padding:10px;
            width: 100%;
            margin-top:20px;
            line-height:30px;
        }
        .hide {
            display:none;
        }
        .login{
            background-color: #00ff00;
            text-decoration: none;
            float: right;
            margin-right: 20px;
        }

    </style>
    </style>
    <title>Sunny</title>
</head>
<body>
<div id = "main" style="text-align: center;">
    <button class="login" id = "li" onclick="logout()" style="width:auto; background-color: #d01d33; color: white">Logout</button>
    <button class="login" id  = "lo" onclick="login()" style="width:auto;">Login</button>
    <p id = "a1">TECHA</p>
    <p id="a2">Taking Executable Command for Home Appliances</p>
    <button type="button" onclick="runSpeechRecognition()" style = "background-color: #fde6ff"> <p id = "a3">Click and then<br>Give you command</p></button> &nbsp; <span id="action"></span>
    <div id="output" class="hide"></div>
    <div style="margin-top: 40px;">
        <div class = "cont1">
            <a href = "#" class = "cont2">ROOM -> 1</a><br><br><br>
            <div class = "cont3">
                <button type="button" onclick="desk_on()"> Turn Desk Light On </button>
                <button type="button" onclick="desk_off()" style = "background-color: #fc8b8b"> Turn Desk Light Off </button>
            </div>
            <div class="cont3"><span id = "state1">###</span></div><br>
            <div class = "cont3">
                <button type="button" onclick="ceil_1_on()">Turn Ceiling Light On </button>
                <button type="button" onclick="ceil_1_off()" style = "background-color: #fc8b8b">Turn Ceiling Light Off </button>
            </div>
            <div class="cont3"><span id = "state2">###</span></div><br>
        </div>
        <div class = "cont1">
            <a href = "#" class = "cont2">ROOM -> 2</a><br><br><br>
            <div class = "cont3">
                <button type="button" onclick="tbl_on()">Turn Table Light On </button>
                <button type="button" onclick="tbl_off()" style = "background-color: #fc8b8b">Turn Table Light Off </button>
            </div>
            <div class="cont3"><span id = "state3">###</span></div><br>
            <div class = "cont3">
                <button type="button" onclick="ceil_2_on()">Turn Ceiling Light On </button>
                <button type="button" onclick="ceil_2_off()" style = "background-color: #fc8b8b">Turn Ceiling Light Off </button>
            </div>
            <div class="cont3"><span id = "state4">###</span></div><br>
        </div>
    </div>
    <button class="login" id  = "nu" onclick="new_u()" style="width:auto; background-color: #045df5; color: white">Add guest login</button>
</div>
    <script>
        var uid = ["sunny", "voldemort"];
        var psd = ["12345", "987654321"];
        var flag = 0;
        var x = document.getElementById("li");
        var y = document.getElementById("lo");
        function login(){
            usd = window.prompt(" User ID: ");
            if (uid.includes(usd)){
                var pwd = window.prompt("Password: ");
                if (psd.includes(pwd)){
                    flag = 1;
                    x.style.display = "none";
                    y.style.display = "block";
                    alert("You have successfully \nlogged in :):)");
                    per = "done";
                }
                else{
                    alert("Incorrect Password :(:(");
                    x.style.display = "block";
                    y.style.display = "none";
                }
            }
            else{
                alert("user id: " + usd + "\nis not registered with us");
                x.style.display = "block";
                y.style.display = "none";
            }
        }
        function logout(){
            flag = 0;
            x.style.display = "block";
            y.style.display = "none";
        }
        function new_u(){
            alert("please login with root account");
            login();
            if (usd == "sunny" && per == "done"){
                var z = window.prompt("user ID for new user: ");
                uid.push(z);
                var z = window.prompt("Corresponding Password: ");
                psd.push(z);
            }
        }
        function desk_on(){
            if (flag == 1) {
                document.getElementById("state1").innnerHTML = "Desk Light in Room 1 is On";
                console.log("Room 1 Desk Light ON");
                connection.send("dl is ON");
            }
            else{
                login();
            }
        }
        function desk_off(){
                if (flag == 1) {
                document.getElementById("state1").innnerHTML = "Desk Light in Room 1 is Off";
                console.log("Room 1 Desk Light is OFF");
                connection.send("dl is OFF");
            }
            else{
                login();
            }
        }
        function ceil_1_on(){
            if (flag == 1) {
                console.log("Room 1 Ceiling Light is ON");
                connection.send("cl1 is ON");
                document.getElementById("state2").innnerHTML="Ceiling Light in Room 1 is On";
            }
            else{
                login();
            }
        }
        function ceil_1_off(){
            if (flag == 1) {
                console.log("Room 1 Ceiling Light is OFF");
                connection.send("cl1 is OFF");
                document.getElementById("state2").innnerHTML="Ceiling Light in Room 1 is Off";
            }
            else{
                login();
            }
        }
        function tbl_on(){
            if (flag == 1) {
                console.log("Table Light is ON");
                connection.send("tl is ON");
                document.getElementById("state3").innnerHTML="Table Light in Room 1 is On";
            }
            else{
                login();
            }
        }
        function tbl_off(){
            if (flag == 1) {
                console.log("Table Light is OFF");
                connection.send("tl is OFF");
                document.getElementById("state3").innnerHTML="Table Light in Room 1 is Off";
            }
            else{
                login();
            }
        }
        function ceil_2_on(){
            if (flag == 1) {
                console.log("Room 2 Ceiling Light is ON");
                connection.send("cl2 is ON");
                document.getElementById("state4").innnerHTML="Ceiling Light in Room 2 is On";
            }
            else{
                login();
            }
        }
        function ceil_2_off(){
            if (flag == 1) {
                console.log("Room 2 Ceiling Light is OFF");
                connection.send("cl2 is OFF");
                document.getElementById("state4").innnerHTML="Ceiling Light in Room 2 is Off";
            }
            else{
                login();
            }
        }
        function runSpeechRecognition() {
            var output = document.getElementById("output");
            var action = document.getElementById("action");
            var SpeechRecognition = SpeechRecognition || webkitSpeechRecognition;
            var recognition = new SpeechRecognition();

            recognition.onstart = function() {
                action.innerHTML = "<small>Listening..........</small>";
            };
            recognition.onspeechend = function() {
                action.innerHTML = "<small>Command Recieved</small>";
                recognition.stop();
            }
            recognition.onresult = function(event) {
                var transcript = event.results[0][0].transcript;
                var confidence = event.results[0][0].confidence;
                output.innerHTML = "<b>Command given: </b> " + transcript + "<b><br> Confidence:</b> " + confidence*100+"%";
                output.classList.remove("hide");
                if (transcript.includes("table" && "on")){
                    tbl_on();
                }
                if (transcript.includes("table" && "of")){
                    tbl_off();
                }
                if (transcript.includes("table" && "off")){
                    tbl_off();
                }
                if (transcript.includes("desk" && "on")){
                    desk_on()
                }
                if (transcript.includes("table" && "of")){
                    desk_off();
                }
                if (transcript.includes("table" && "off")){
                    desk_off();
                }
                if (transcript.includes("ceiling" && "one" && "on")){
                    ceil_1_on()
                }
                if (transcript.includes("ceiling" && "1" && "on")){
                    ceil_1_on()
                }
                if (transcript.includes("ceiling" && "one" && "of")){
                    ceil_1_off()
                }
                if (transcript.includes("ceiling" && "one" && "off")){
                    ceil_1_off()
                }
                if (transcript.includes("ceiling" && "1" && "of")){
                    ceil_1_off()
                }
                if (transcript.includes("ceiling" && "1" && "off")){
                    ceil_1_off()
                }
                if (transcript.includes("ceiling" && "two" && "on")){
                    ceil_2_on()
                }
                if (transcript.includes("ceiling" && "2" && "on")){
                    ceil_2_on()
                }
                if (transcript.includes("ceiling" && "to" && "on")){
                    ceil_2_on()
                }
                if (transcript.includes("ceiling" && "two" && "of")){
                    ceil_2_off()
                }
                if (transcript.includes("ceiling" && "two" && "off")){
                    ceil_2_off()
                }
                if (transcript.includes("ceiling" && "2" && "of")){
                    ceil_2_off()
                }
                if (transcript.includes("ceiling" && "2" && "off")){
                    ceil_2_off()
                }
                if (transcript.includes("ceiling" && "to" && "of")){
                    ceil_2_off()
                }
                if (transcript.includes("ceiling" && "to" && "off")){
                    ceil_2_off()
                }
            };
            recognition.start();
        }

    </script>
</body>
</html>
)=====";
  
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Page Not found");
}


void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

  switch (type) 
  {
    case WStype_DISCONNECTED:
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {
        IPAddress ip = websockets.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

        // send message to client
        websockets.sendTXT(num, "Connected from server");
      }
      break;
    case WStype_TEXT:
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String message = String((char*)( payload));
      Serial.println(message);

      if(message == "cl1 is OFF"){
        digitalWrite(cl1,LOW);
      }
      if(message == "cl1 is ON"){
        digitalWrite(cl1,HIGH);
      }
      if(message == "cl2 is OFF"){
        digitalWrite(cl2,LOW);
      }
      if(message == "cl2 is ON"){
        digitalWrite(cl2,HIGH);
      }
      if(message == "tl is OFF"){
        digitalWrite(tl,LOW);
      }
      if(message == "tl is ON"){
        digitalWrite(tl,HIGH);
      }
      if(message == "dl is OFF"){
        digitalWrite(dl,LOW);
      }
      if(message == "dl is ON"){
        digitalWrite(dl,HIGH);
      }
  }
}

void setup(void)
{
  
  Serial.begin(115200);
  pinMode(tl,OUTPUT);
  pinMode(dl,OUTPUT);
  pinMode(cl1,OUTPUT);
  pinMode(cl2,OUTPUT);
  
  WiFi.softAP("my_socket", "qwerty");
  Serial.println("launching web-socket");
  Serial.println("");
  Serial.println(WiFi.softAPIP());


  if (MDNS.begin("ESP")) { //esp.local/
    Serial.println("MDNS responder started");
  }



  server.on("/", [](AsyncWebServerRequest * request)
  { 
   
  request->send_P(200, "text/html", webpage);
  });

//   server.on("/led1/on", HTTP_GET, [](AsyncWebServerRequest * request)
//  { 
//    digitalWrite(LED1,HIGH);
//  request->send_P(200, "text/html", webpage);
//  });
//  server.on("/led2/on", HTTP_GET, [](AsyncWebServerRequest * request)
//  { 
//    digitalWrite(LED2,HIGH);
//  request->send_P(200, "text/html", webpage);
//  });

  server.onNotFound(notFound);

  server.begin(); 
  websockets.begin();
  websockets.onEvent(webSocketEvent);

}


void loop(void)
{
 websockets.loop();
}
