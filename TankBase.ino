#include <math.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


//motor speed pins
#define MR_SPEED_PIN D15
#define ML_SPEED_PIN D12

//motor direction pins
#define MR_DIR_PIN D14
#define ML_DIR_PIN D13


#define MAX_SPEED 1023
#define MIN_SPEED 700

//from led pin
#define FRNT_LED_PIN D0


IPAddress local_IP(192,168,4,1);
IPAddress gateway(192,168,4,138);
IPAddress subnet(255,255,255,0);

ESP8266WebServer server(80);

void handleRoot();              // function prototypes for HTTP handlers
void handleNotFound();

boolean frontLedOn = false;
void setupPins() {
  Serial.println("Setting up motors!");
  pinMode(MR_SPEED_PIN, OUTPUT);
  pinMode(ML_SPEED_PIN, OUTPUT);
  pinMode(MR_DIR_PIN, OUTPUT);
  pinMode(ML_DIR_PIN, OUTPUT);

  pinMode(FRNT_LED_PIN, OUTPUT);
  
}

void move(float r, float l) {
  int r_dir = (r>=0) ? HIGH : LOW;
  int l_dir = (l>=0) ? HIGH : LOW;

  float r_speed = ( r==0 ) ? 0 : ((MAX_SPEED - MIN_SPEED) * fabs(r)) + MIN_SPEED;
  float l_speed = ( l==0 ) ? 0 : ((MAX_SPEED - MIN_SPEED) * fabs(l)) + MIN_SPEED;
  digitalWrite(MR_DIR_PIN, r_dir);
  digitalWrite(ML_DIR_PIN, l_dir);
  analogWrite(MR_SPEED_PIN, r_speed);
  analogWrite(ML_SPEED_PIN, l_speed);
  Serial.println("SPEED R\t|\tSPEED L");
  Serial.print(r_speed);
  Serial.print("\t|\t");
  Serial.println(l_speed);
}

void turnFrontLedOn() {
  frontLedOn = true;
  digitalWrite(FRNT_LED_PIN, HIGH);
}

void turnFrontLedOff() {
  digitalWrite(FRNT_LED_PIN, LOW);
  frontLedOn = false;
}

void setupSoftAp() {
  
  Serial.print("Setting soft-AP configuration ... ");
  Serial.println(WiFi.softAPConfig(local_IP, gateway, subnet) ? "Ready" : "Failed!");

  Serial.print("Setting soft-AP ... ");
  boolean result = WiFi.softAP("ESPsoftAP_01", "pass-to-soft-AP", 1, false, 8);
  if(result == true)
  {
    Serial.println("Ready");
  }
  else
  {
    Serial.println("Failed!");
  }
  
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}
void setup()
{
  Serial.begin(115200);
  delay(1000);
  Serial.println();

  setupPins();
  setupSoftAp();
  server.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"
  server.on("/drive", [] () {
    if(!server.hasArg("direction")) { // If the POST request doesn't have username and password data
      server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
      return;
    }
    String dir = server.arg("direction");
    float speedr = 0, speedl = 0;
    sscanf(dir.c_str(), "(%f,%f)", &speedl, &speedr);
    move(speedr,speedl);
    server.send(200, "text/html", "ok");
  });
  server.on("/front_led", [] () {
    if(!server.hasArg("state")) { // If the POST request doesn't have username and password data
      server.send(400, "text/plain", "400: Invalid Request");         // The request is invalid, so send HTTP status 400
      return;
    }
    String state = server.arg("state");
    Serial.println(state);
    if(state == "on") {
      turnFrontLedOn();
    } else {
      turnFrontLedOff();
    }
    server.send(200, "text/html", "ok");
  });
  server.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  server.begin();                           // Actually start the server
  Serial.println("HTTP server started");
}
void loop()
{
  server.handleClient();                     // Listen for HTTP requests from clients

}

void handleRoot() {
  server.send(200, "text/plain", "Hello world!");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
  server.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}
