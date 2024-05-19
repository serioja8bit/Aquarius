#ifndef FINGERPRINTMANAGER_H
#define FINGERPRINTMANAGER_H

#include <Adafruit_Fingerprint.h>

enum FingerState {
    menu,
    dafault,
    waiting,
    firstRead,
    secondRead,
    enrolling,
    verifying,
    ok,
    fail,
    

};

extern Adafruit_Fingerprint finger; // Declare external reference to fingerprint object
extern uint8_t id;
extern FingerState fingerState;
uint8_t getFingerprintEnroll(uint8_t id);
uint8_t getFingerprintID(void);
uint8_t countStoredFingerprints(void);
void OK_STAT(void);
void FAIL_STAT(void);
void WAIT_STAT(void);
void SECOND_READ_STAT(void);
void FIRST_READ_STAT(void);
#endif
