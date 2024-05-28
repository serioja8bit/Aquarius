#include "FingerprintManager.h"
#include "KeyboardManager.h"
#include <SPI.h>
#include <TFT_eSPI.h>
#include <Adafruit_ILI9341.h>

#define BUTTON_HEIGHT 60
#define BUTTON_WIDTH 120
#define BUTTON_GAP 20

#define SCREEN_WIDTH 320
#define TOTAL_BUTTONS_WIDTH (2 * BUTTON_WIDTH + BUTTON_GAP)
#define START_X ((SCREEN_WIDTH - TOTAL_BUTTONS_WIDTH) / 2)

#define BUTTON1_X START_X
#define BUTTON1_Y 80
#define BUTTON2_X (BUTTON1_X + BUTTON_WIDTH + BUTTON_GAP)
#define BUTTON2_Y BUTTON1_Y

struct TouchPoint {
  uint16_t x;
  uint16_t y;
  bool valid;
};

boolean Caps = false;
String currentText = "";
String name = "";
String age;
String weight;
String symbol[4][10] = {
  { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" },
  { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p" },
  { "a", "s", "d", "f", "g", "h", "j", "k", "l", "Enter" },
  { "Caps", "z", "x", "c", "v", "b", "n", "m", ".", "<" }
};

#define mySerial Serial2

#define WATER_FLOW_SENSOR_PIN 34
volatile int flowPulseCount = 0;
float totalWater = 0;
float waterFlow = 0;
const float calibrationFactor = 4.5;
const int sampleTime = 1000;

bool isTouchWithinArea(TouchPoint p, int x, int y, int width, int height);
void drawMenu();
TouchPoint getTouchCoordinates();
uint8_t readnumber(void);

uint8_t id = 0;
uint8_t action;

TFT_eSPI tft = TFT_eSPI();
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

void setup() {
  Serial.begin(9600);
  while (!Serial);
  delay(100);
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getParameters();
  Serial.print("Status: 0x"); Serial.println(finger.status_reg, HEX);
  Serial.print("Sys ID: 0x"); Serial.println(finger.system_id, HEX);
  Serial.print("Capacity: "); Serial.println(finger.capacity);
  Serial.print("Security level: "); Serial.println(finger.security_level);
  Serial.print("Device address: 0x"); Serial.println(finger.device_addr, HEX);
  Serial.print("Packet len: "); Serial.println(finger.packet_len);
  Serial.print("Baud rate: "); Serial.println(finger.baud_rate);
  Serial.print("Stored fingers: "); Serial.println(countStoredFingerprints());
  id = countStoredFingerprints();
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.println("AquariusMK2");

  uint16_t calData[5] = { 417, 3384, 359, 3388, 3 };
  tft.setTouch(calData);
}

void loop() {
  drawMenu();
  while (fingerState == menu) {
    TouchPoint touchPoint = getTouchCoordinates();
    if (touchPoint.valid) {
      if (isTouchWithinArea(touchPoint, BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
        action = 1;
        Serial.println("Enroll button pressed");
        fingerState = waiting;
      } else if (isTouchWithinArea(touchPoint, BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
        action = 2;
        Serial.println("Verify button pressed");
        fingerState = waiting;
      } else {
        action = -1;
      }
    }
  }

  switch (action) {
    case 1:
      fingerState = waiting;
      drawMenu();
      id++;
      if (id == 0) return;
      Serial.print("Enrolling ID #"); Serial.println(id);
      while (!getFingerprintEnroll(id));
      path = "Persons/"+ id;
      draw_Keyboard();
      name = checkTouch("Name");
      age = checkTouch("Age");
      weight = checkTouch("Weight");
      drawInputFields(name, age, weight);
      delay(10000);
      fingerState = menu;
      action = -1;
      break;
    case 2:
      Serial.println("Put your finger");
      fingerState = waiting;
      drawMenu();
      id = 0;
      while (!(id = getFingerprintID()));
      Serial.println(id);
      fingerState = menu;
      action = -1;
      break;
    default:
      break;
  }
  delay(20);
}

uint8_t readnumber() {
  uint8_t num = 0;
  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

TouchPoint getTouchCoordinates() {
  TouchPoint touchPoint;
  uint16_t z = 600U;
  touchPoint.valid = tft.getTouch(&touchPoint.x, &touchPoint.y, z);
  return touchPoint;
}

void drawMenu() {
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(10, 10);

  switch (fingerState) {
    case waiting:
      tft.print("Please place your finger");
      break;
    case firstRead:
      tft.print("Reading finger...");
      break;
    case secondRead:
      tft.print("Please lift and place finger again");
      break;
    case verifying:
      tft.print("Verifying, please wait...");
      break;
    case ok:
      tft.setTextColor(TFT_GREEN);
      tft.print("Fingerprint verified!");
      break;
    case fail:
      tft.setTextColor(TFT_RED);
      tft.print("Verification failed!");
      break;
    case menu:
      tft.fillRect(BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_BLUE);
      tft.fillRect(BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_GREEN);

      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(2);
      tft.setCursor(BUTTON1_X + (BUTTON_WIDTH - tft.textWidth("Enroll")) / 2, BUTTON1_Y + (BUTTON_HEIGHT - 16) / 2);
      tft.print("Enroll");
      tft.setCursor(BUTTON2_X + (BUTTON_WIDTH - tft.textWidth("Verify")) / 2, BUTTON2_Y + (BUTTON_HEIGHT - 16) / 2);
      tft.print("Verify");
      break;
  }
}

bool isTouchWithinArea(TouchPoint p, int x, int y, int width, int height) {
  return (p.x >= x && p.x <= x + width && p.y >= y && p.y <= y + height);
}
