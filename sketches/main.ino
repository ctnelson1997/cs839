#include <WiFi.h>
#include <HTTPClient.h>
//#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include "esp_camera.h"
#include "Base64.h"

#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
#define MQ3Analog 33 // MQ-3 analog
#define echoPin 12   // Ultrasonic EchoPin
#define trigPin 32   // Ultrasonic TrigPin
//#define SDA 21 // Define SDA pins
//#define SCL 22 // Define SCL pins
#define SOUND 15 // Piezo buzzer pins
#define Sober 120   // Define max value that we consider not Drunk
#define Drunk 400   // Define min value that we consider drunk
#define MAX_DISTANCE 800 // Maximum sensor distance is rate at 400-500 cm
#define DISTANCE_THRESHOLD 50

#include "camera_pins.h"

const char* ssid = "YOUR WIFI SSID";
const char* password = "YOUR WIFI Password";

bool connect_flag = false;

const char* server_name = "http://maker.ifttt.com/trigger/detect_alcohol/with/key/cmScmmVh9LNuU-3j3prORj";

String myFilename = "filename=ESP32-CAM.png";
String mimeType = "&mimetype=data:image/png;base64,";
String myImage = "&data=";

//int sensorPin = 14; // the number of the infrared motion sensor pin
int ledPinRed = 13;    // the number of the LED pin
int ledPinGreen = 14;
int ledPinBlue = 2; // onboard LED
int buzzerPin = SOUND; // the number of the buzzer pin
float sinVal;          // Define a variable to save sine value
int toneVal;           // Define a variable to save sound frequency
int pinStateCurrent = LOW;
int pinStatePrevious = LOW;
float duration_us, distanceCurrent, distancePrevious;

float drunken = 0;
int drunken_step = 0;
char drunken_str[10];

camera_config_t config;

// initialize the library with the numbers of the interface pins
/*
 * note:If lcd1602 uses PCF8574T, IIC's address is 0x27,
 *      or lcd1602 uses PCF8574AT, IIC's address is 0x3F.
*/
//LiquidCrystal_I2C lcd(0x27, 16, 2);

void config_init();
String url_encode();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println("MQ3 is warming up");
  delay(20000); // 20 seconds warm up time

  // Display initialization
//  Wire.begin(SDA, SCL); // attach the IIC pin
//  lcd.init();                     // LCD driver initialization
//  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
//  lcd.backlight();
//  lcd.print("hello, world!");     // The print content is displayed on the LCD
//  
  // initialize camera config
  config_init();

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

//  sensor_t * s = esp_camera_sensor_get();
//  s->set_vflip(s, 0);        //1-Upside down, 0-No operation
//  s->set_hmirror(s, 0);      //1-Reverse left and right, 0-No operation
//  s->set_brightness(s, 1);   //up the blightness just a bit
//  s->set_saturation(s, -1);  //lower the saturation

  WiFi.begin(ssid, password);
  Serial.println("Connecting to WIFI...");
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("After 10 seconds the first set of readings will be displayed");

//  pinMode(sensorPin, INPUT);     // initialize the sensor pin as input
  pinMode(ledPinRed, OUTPUT);    // initialize the LED pin as output
  pinMode(ledPinGreen, OUTPUT);  // initialize the LED pin as output
  pinMode(ledPinBlue, OUTPUT);   // initialize the LED pin as output
  pinMode(buzzerPin, OUTPUT);    // Set Buzzer pin to output mode
  pinMode(trigPin, OUTPUT);      // Set trigPin to output mode
  pinMode(echoPin, INPUT);       // Set echoPin to input mode
}


//TODO: 
//5. Make cover
void loop() {
//   Turn on or off LED according to Infrared Motion Sensor
//  pinStatePrevious = pinStateCurrent;
//  pinStateCurrent = digitalRead(sensorPin);

//  lcd.setCursor(0,0);             // Move the cursor to row 0, column 0
//  lcd.print("Get ready for BCD Test");     // The print content is displayed on the LCD
//  lcd.print("Hello, World!");

  // Trigger MQ-3 and collect data
  float mq3_value = analogRead(MQ3Analog);
  Serial.print("MQ-3 Sensor Value: ");
  Serial.println(mq3_value);
  sprintf(drunken_str, "%f", mq3_value);

  // Distance
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // measure duration of pulse from ECHO pin
  duration_us = pulseIn(echoPin, HIGH);
  // calculate the distance
  distanceCurrent = 0.017 * duration_us;

  Serial.printf("Distance: ");
  Serial.print(distanceCurrent);
  Serial.println("cm");
  
//  if (pinStatePrevious == LOW && pinStateCurrent == HIGH){ // This is for motion sensor
  if ((drunken_step == 0) && (distanceCurrent < 50) && (abs(distancePrevious - distanceCurrent) < DISTANCE_THRESHOLD)){
    connect_flag = true;
    digitalWrite(ledPinBlue, HIGH);
    Serial.println("Person detected!");
    drunken_step = 10;
  } else if ((distanceCurrent < 50) && (abs(distancePrevious - distanceCurrent) < DISTANCE_THRESHOLD)){
    digitalWrite(ledPinBlue, HIGH);
    Serial.println("Person detected!");
  }
//  else if (pinStatePrevious == HIGH && pinStateCurrent == LOW){
  else {
    drunken_step = (drunken_step < 0) ? 0 : drunken - 3; // If nobody is in front, it gets faster to take next photo
    Serial.println("Nobody here");
    digitalWrite(ledPinBlue, LOW);
    connect_flag = false;
  }

 // Check WiFi connection status
  if(WiFi.status() == WL_CONNECTED){
    if (mq3_value < Sober) {
      Serial.println("  |  Status: SOBER");
      digitalWrite(ledPinGreen, HIGH);
    } else if (mq3_value >= Sober && mq3_value < Drunk) {
      digitalWrite(ledPinGreen, HIGH);
      Serial.println("  |  Status: Drinking but within legal limits");
    } else {
      HTTPClient http;
      http.begin(server_name);
      
      Serial.println("  |  Status: DRUNK");
      digitalWrite(ledPinRed, HIGH);
      if (connect_flag == true){
        myTone(buzzerPin);
        delay(1000);
        myNoTone(buzzerPin); 
        delay(5000);    // Shoot after 5 seconds
        // Set up image
        camera_fb_t * fb = NULL;
        
        // Capture image
        fb = esp_camera_fb_get();
    
        if(!fb) {
          Serial.println("Camera capture failed");
          delay(1000);
          ESP.restart();
          return;
        }
    
        char *input = (char*) fb->buf;
        char output[base64_enc_len(3)];
        String imageFile = "";
        imageFile += "data:image/png;base64,"; // add mimeType
        for (int i=0; i<fb->len; i++){
          base64_encode(output, (input++), 3);
          if (i%3==0) imageFile += urlencode(String(output));
        }
    
        String imgData = imageFile;
    
        esp_camera_fb_return(fb);
        
        // Specify content-type header
        http.addHeader("Content-Type", "application/x-www-form-urlencoded");
        // Data to send with HTTP POST
        String httpRequestData = "value1=" + String(drunken_str) + "&value2=" + '"' + imgData + '"';
        // Send HTTP POST request
        int httpResponseCode = http.POST(httpRequestData);

        Serial.println("Picture taken");
    
        Serial.print("HTTP Response code is: ");
        Serial.println(httpResponseCode);

        http.end();
        connect_flag = false;
      }
    }
    drunken_step = (drunken_step < 0) ? 0 : drunken_step - 1;
  } else {
    Serial.println("WiFi Disconnected");
  }

  distancePrevious = distanceCurrent;

  delay(2000);              // wait for a second

  // Reset
  digitalWrite(ledPinGreen, LOW);
  digitalWrite(ledPinRed, LOW);
  delay(1000);
}

// https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}

void config_init() {
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;
}

void myTone( int pin)
{
  ledcAttachPin(pin, 0);          // pin, channel
  ledcWriteNote(0, NOTE_F, 4);    // channel, frequency, octave
}

void myNoTone( int pin)
{
  ledcDetachPin(pin);
}
