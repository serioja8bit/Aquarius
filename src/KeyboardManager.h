#include "FS.h"
#include <SPI.h>
#include <TFT_eSPI.h>

void draw_BoxNButtons(void);
void checkTouch(void);
void updateDisplay(void);
extern TFT_eSPI tft; 


//uint16_t X = 0, Y = 0; 
extern boolean Caps;
extern String currentText;
extern String symbol[4][10];