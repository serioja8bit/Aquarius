
#include "KeyboardManager.h"



void draw_Keyboard() {
    int i,j;
    tft.fillScreen(TFT_BLACK);
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED);
    // Draw all keys including the new numeric row
    for (j = 0; j < 4; j++) {
        for (i = 0; i < 10; i++) {
            tft.fillRoundRect((i * 32) + 1, j * 35 + 66, 31, 34, 3, TFT_YELLOW);  // Adjust Y offset to 66
            tft.setCursor(i * 32 + 9, j * 35 + 72);
            tft.print(symbol[j][i]);
        }
    }

 
      tft.fillRoundRect(1,171, 31, 34, 3,TFT_CYAN);
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize (1);
      tft.setCursor (5,185 );
      tft.print("Caps");
      tft.fillRoundRect(289,171, 31, 34, 3,0Xfbef);
      tft.setTextColor(TFT_BLACK);
      tft.setTextSize (2);
      tft.setCursor (290,182 );
      tft.print("<-");

      tft.fillRoundRect(289,136, 31, 34, 3,TFT_CYAN);
      tft.setTextColor(TFT_RED);
      tft.setTextSize (1);
      tft.setCursor (290,150 );
      tft.print("Enter");
      
      tft.fillRoundRect(1,206, 223, 34, 3,TFT_VIOLET);
      tft.setTextColor(TFT_WHITE);
      tft.setTextSize (2);
      tft.setCursor (55,215 );
      tft.print("Space Bar");

         
      tft.fillRoundRect(225,206, 95, 34, 3,TFT_WHITE);
      tft.setTextColor(TFT_VIOLET);
      tft.setTextSize (2);
      tft.setCursor (247,217 );
      tft.print("RESET");
}

String checkTouch(String str) {
    uint16_t x, y;
    currentText = ""; // reset current text
    tft.fillRect(0, 0, 320, 66, TFT_BLACK);
    tft.setCursor(10, 10);  // Set text cursor within the cleared area
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.print(str + ": " + currentText);  // Display the current text
    while(true){
        if (tft.getTouch(&x, &y)) {
        // Ajustează calculele pentru keyRow pentru a reflecta noile poziții ale rândurilor de butoane
        int keyRow = (y - 66) / 35;  // Ajustează valoarea de start a Y de la 101 la 66
        int keyCol = x / 32;

        if (keyRow >= 0 && keyRow < 4 && keyCol >= 0 && keyCol < 10) {
            String key = symbol[keyRow][keyCol];
            if (key == "Enter") {
                return currentText;
                delay(100);
            } else if (key == "Caps") {
                Caps = !Caps;
            }else if (key == "<") {
                if (currentText.length() > 0) {
                    currentText.remove(currentText.length() - 1);
                }
            } else {
                if (Caps && key != "Space Bar" && key != "RESET") {
                    key.toUpperCase();
                }
                currentText += key;
            }
            Serial.println(currentText);
            // Adjust the fillRect to clear a smaller area, avoiding the top row of keys
            tft.fillRect(0, 0, 320, 66, TFT_BLACK);  // Reduce height to 66
            // Update the text display area accordingly
            tft.setCursor(10, 10);  // Set text cursor within the cleared area
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(2);
            tft.print(str + ": " + currentText);  // Display the current text
            delay(150);
            
        }
         
    }
        
    }
    
}



void updateDisplay() {
    // Adjust the fillRect to clear a smaller area, avoiding the top row of keys
    tft.fillRect(0, 0, 320, 66, TFT_BLACK);  // Reduce height to 66

    // Update the text display area accordingly
    tft.setCursor(10, 10);  // Set text cursor within the cleared area
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.print("Prefix:" + currentText);  // Display the current text
}

void drawInputFields(String name, String age, String weight) {
    
    tft.fillScreen(TFT_BLACK);
    tft.fillRect(20, 20, 280, 40, TFT_WHITE);  // Name field
    tft.fillRect(20, 80, 280, 40, TFT_WHITE);  // Age field
    tft.fillRect(20, 140, 280, 40, TFT_WHITE); // Weight field
    tft.setTextColor(TFT_BLACK);
    tft.setCursor(30, 30);
    tft.print("Name: "+ name);
    tft.setCursor(30, 90);
    tft.print("Age: "+ age);
    tft.setCursor(30, 150);
    tft.print("Weight: "+ weight);
        
}
