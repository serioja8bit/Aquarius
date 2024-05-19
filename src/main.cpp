#include "FingerprintManager.h"
#include "KeyboardManager.h"
#include <SPI.h>
#include <TFT_eSPI.h>      // Hardware-specific library
// #include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
//#include <XPT2046_Touchscreen.h>

#define BUTTON_HEIGHT 60
#define BUTTON_WIDTH 120
#define BUTTON_GAP 20

// Calcularea centrării butoanelor pe ecran
#define SCREEN_WIDTH 320
#define TOTAL_BUTTONS_WIDTH (2 * BUTTON_WIDTH + BUTTON_GAP)
#define START_X ((SCREEN_WIDTH - TOTAL_BUTTONS_WIDTH) / 2)

// Button areas
#define BUTTON1_X START_X
#define BUTTON1_Y 80
#define BUTTON2_X (BUTTON1_X + BUTTON_WIDTH + BUTTON_GAP)
#define BUTTON2_Y BUTTON1_Y

struct TouchPoint {
  uint16_t x;
  uint16_t y;
  bool valid; // Indicates if the touch coordinates are valid
};
boolean Caps = false;
String currentText = "";
String symbol[4][10] = {
  { "1", "2", "3", "4", "5", "6", "7", "8", "9", "0" }, // New numeric row
  { "q", "w", "e", "r", "t", "y", "u", "i", "o", "p" },
  { "a", "s", "d", "f", "g", "h", "j", "k", "l", "Enter" },
  { "Caps", "z", "x", "c", "v", "b", "n", "m", ".", "<" }
};

#define mySerial Serial2

/*===Water Flow Part===*/
#define WATER_FLOW_SENSOR_PIN 34 // Example pin
volatile int flowPulseCount = 0; // Pulse count from the sensor
// Constants for flow rate calculation
float totalWater = 0;
float waterFlow = 0;
const float calibrationFactor = 4.5; // Calibration factor for the sensor
const int sampleTime = 1000; // Sample time in milliseconds
/*===Prototype===*/
void waterFlowTask(void *pvParameters);
void fingerprintTask(void *pvParameters);
void touchMenuTask(void *pvParameters);
bool isTouchWithinArea(TouchPoint p, int x, int y, int width, int height);
void drawMenu();
TouchPoint getTouchCoordinates();
uint8_t readnumber(void);


uint8_t id = 0;
uint8_t action;

TFT_eSPI tft = TFT_eSPI(); // Invoke custom library
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
FingerState fingerState;

void setup() {
  Serial.begin(9600);
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
   // Set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x"));        Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));        Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));        Serial.println(finger.capacity);
  Serial.print(F("Security level: "));  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: "));      Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));       Serial.println(finger.baud_rate);
  Serial.print("there are fingers stored"); Serial.println(countStoredFingerprints());

  id = countStoredFingerprints();
  
  /*===TFT init part===*/
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.println("AquariusMK2");
  //calibration
  // Use this calibration code in setup():
  uint16_t calData[5] = { 417, 3384, 359, 3388, 3 };
  tft.setTouch(calData);
}

void loop() {
  drawMenu();
  while(1)
  {
    TouchPoint touchPoint = getTouchCoordinates();
    if (touchPoint.valid) {
      if (isTouchWithinArea(touchPoint, BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
        // Enroll action
        action = 1;
        Serial.println("Enroll button pressed");
        draw_BoxNButtons();
        break;
      } else if (isTouchWithinArea(touchPoint, BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
        // Verify action
        action = 2;
        Serial.println("Verify button pressed");
        fingerState = waiting;
        break;
      } else {
        action = -1;
      }
    }
  }
    //drawMenu();
    switch (action) {
      case 1:
        Serial.println("Ready to enroll a fingerprint!");
        id++;
        if (id == 0) {
            return;
        }
        Serial.print("Enrolling ID #");
        Serial.println(id);
        while (!getFingerprintEnroll(id))
        {
          drawMenu();
          delay(1000);
        }
        action = -1;
        break;
      case 2:
        //drawMenu();
        Serial.println("Put your finger");
        id = 0;
        while (!(id = getFingerprintID()))
        {
          drawMenu();
          delay(1000);
        }
        Serial.println(id);
        action = -1;
        break;
      default:
        break;
    }
    delay(20); // Adjust delay as needed
}

// void IRAM_ATTR flowPulseCounter() {
//   flowPulseCount++;
// }

// void waterFlowTask(void *pvParameters) {
//   pinMode(WATER_FLOW_SENSOR_PIN, INPUT_PULLUP); // Set the sensor pin as input
//   attachInterrupt(digitalPinToInterrupt(WATER_FLOW_SENSOR_PIN), flowPulseCounter, RISING); // Attach the interrupt

//   while (1) {
//     int count = flowPulseCount; // Get the current pulse count
    

//     float flowRate = (count / calibrationFactor) * (1000.0 / sampleTime); // Calculate the flow rate in L/min
//     float volume = flowRate * (sampleTime / 60000.0); // Calculate the volume in liters
//     totalWater += volume;
//     waterFlow  = flowRate;
//     Serial.print("Flow rate: ");
//     Serial.print(flowRate);
//     Serial.println(" L/min");
//     Serial.print("Volume: ");
//     Serial.print(volume);
//     Serial.println(" L");
//     flowPulseCount = 0; // Reset the pulse count

//     vTaskDelay(sampleTime / portTICK_PERIOD_MS); // Wait for the next sample
//   }
// }



uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (!Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

TouchPoint getTouchCoordinates() {
  TouchPoint touchPoint;
  uint16_t z = 600U; // Variable to hold the pressure value
  touchPoint.valid = tft.getTouch(&touchPoint.x, &touchPoint.y, z);
  return touchPoint;
}


void drawMenu() {
     switch(fingerState) {
      case waiting:
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.print("Please place your finger");
        break;
      case firstRead:
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.print("Reading finger...");
        break;
      case secondRead:
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.print("Please lift and place finger again");
        break;
      case verifying:
        tft.fillScreen(ILI9341_BLACK);
        tft.setCursor(10, 10);
        tft.print("Verifying, please wait...");
        break;
      case ok:
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(TFT_GREEN);
        tft.setCursor(10, 10);
        tft.print("Fingerprint verified!");
        break;
      case fail:
        tft.fillScreen(ILI9341_BLACK);
        tft.setTextColor(TFT_RED);
        tft.setCursor(10, 10);
        tft.print("Verification failed!");
        break;
      case menu:
        // Clear the display or draw the background first if needed
      tft.fillScreen(ILI9341_BLACK);

      // Draw the menu buttons
      tft.fillRect(BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_BLUE);
      tft.fillRect(BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT, ILI9341_GREEN);

      // Add text labels for each button
      tft.setTextColor(ILI9341_WHITE);
      tft.setTextSize(2);
      tft.setCursor(BUTTON1_X + (BUTTON_WIDTH - tft.textWidth("Enroll")) / 2, BUTTON1_Y + (BUTTON_HEIGHT - 16) / 2);  // Centering "Enroll"
      tft.print("Enroll");
      tft.setCursor(BUTTON2_X + (BUTTON_WIDTH - tft.textWidth("Verify")) / 2, BUTTON2_Y + (BUTTON_HEIGHT - 16) / 2); // Centering "Verify"
      tft.print("Verify");
      break;
    }
}

bool isTouchWithinArea(TouchPoint p, int x, int y, int width, int height) {
  
  return (p.x >= x && p.x <= x + width && p.y >= y && p.y <= y + height);
}
