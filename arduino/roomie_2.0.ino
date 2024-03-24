#include <usbhid.h> // biblioteca Arduino pentru UsbHostShield
#include <hiduniversal.h> // biblioteca Arduino pentru UsbHostShield
#include <usbhub.h> // biblioteca Arduino pentru UsbHostShield 
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include "hidjoystickrptparser.h" // biblioteca Arduino pentru UsbHostShield 

// crearea variabilelor necesare conectarii cu joystick-ul
USB Usb;
USBHub Hub(&Usb);
HIDUniversal Hid(&Usb);
JoystickEvents JoyEvents;
JoystickReportParser Joy(&JoyEvents);

// crearea variabilelor necesare pentru algoritmul de miscare cu ajutorul joystick-ului
int inFata = 0, inFataOld = 0;
int inSpate = 0, inSpateOld = 0;
int inStanga = 0, inStangaOld = 0;
int inDreapta = 0, inDreaptaOld = 0;
int rotStanga = 0, rotStangaOld = 0;
int rotDreapta = 0, rotDreaptaOld = 0;
int spateStanga = 0, spateStangaOld = 0;
int spateDreapta = 0, spateDreaptaOld = 0;

const int stFront = 4; //motor stanga fata
const int stBack = 5  ;//motor stanga spate
const int drBack = 2  ;//motor dreapta spate
const int drFront = 3 ;//motor dreapta fata

//setup se apeleaza o singura data
void setup() {
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial)
    ;  // pentru afisare de date
#endif
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay(200);

  if (!Hid.SetReportParser(0, &Joy))
    ErrorMessage<uint8_t >(PSTR("SetReportParser"), 1);

  //setarea pinilor
  pinMode(stFront, OUTPUT);
  pinMode(stBack, OUTPUT);
  pinMode(drBack, OUTPUT);
  pinMode(drFront, OUTPUT);
}

//functie pentru cand butoanele de la joystick sunt apasate
void butonulApasat(int but) {
  //butonul 1 este "X"
  if (but == 1)
    inStanga = 1;
  //butonul 2 este "A"
  if (but == 2)
    inSpate = 1;
  //butonul 3 este "B"
  if (but == 3)
    inDreapta = 1;
  //butonul 4 este "Y"
  if (but == 4)
    inFata = 1;
  //butonul stanga sus
  if (but == 5)
    spateStanga = 1;
  //butonul dreapta sus
  if (but == 6)
    spateDreapta = 1;
  //butonul stanga jos
  if (but == 7)
    rotStanga = 1;
  //butonul dreapta jos
  if (but == 8)
    rotDreapta = 1;
}

//functie pentru cand butoanele de la joystick sunt ridicate
void butonulRidicat(int but) {
  if (but == 1)
    inStanga = 0;
  if (but == 2)
    inSpate = 0;
  if (but == 3)
    inDreapta = 0;
  if (but == 4)
    inFata = 0;
  if (but == 5)
    spateStanga = 0;
  if (but == 6)
    spateDreapta = 0;
  if (but == 7)
    rotStanga = 0;
  if (but == 8)
    rotDreapta = 0;
}
 //functie care opreste toate motoarele
void gata() {
  Serial.println("stop");
  digitalWrite(drFront, LOW);
  digitalWrite(stFront, LOW);
  digitalWrite(drBack, LOW);
  digitalWrite(stBack, LOW);
}
 //functie care misca robotul in fata
void forward() {
  Serial.println("inFata");
  digitalWrite(drFront, HIGH);
  digitalWrite(stFront, HIGH);
  digitalWrite(drBack, LOW);
  digitalWrite(stBack, LOW);
}
//functie care misca robotul in spate
void backwards() {
  Serial.println("inSpate");
  digitalWrite(drFront, LOW);
  digitalWrite(stFront, LOW);
  digitalWrite(drBack, HIGH);
  digitalWrite(stBack, HIGH);
}
//functie care misca robotul spre dreapta, in fata
void frontRight() {
  Serial.println("inDreapta");
  digitalWrite(drFront, LOW);
  digitalWrite(stFront, HIGH);
  digitalWrite(drBack, LOW);
  digitalWrite(stBack, LOW);
}
//functie care misca robotul spre stanga, in fata
void frontLeft() {
  Serial.println("inStanga");
  digitalWrite(drFront, HIGH);
  digitalWrite(stFront, LOW);
  digitalWrite(drBack, LOW);
  digitalWrite(stBack, LOW);
}
//functie care misca robotul spre dreapta, in spate
void backRight() {
  Serial.println("inDreapta");
  digitalWrite(drFront, LOW);
  digitalWrite(stFront, LOW);
  digitalWrite(drBack, LOW);
  digitalWrite(stBack, HIGH);
}
//functie care misca robotul spre stanga, in spate
void backLeft() {
  Serial.println("inStanga");
  digitalWrite(drFront, LOW);
  digitalWrite(stFront, LOW);
  digitalWrite(drBack, HIGH);
  digitalWrite(stBack, LOW);
}
//functie care misca robotul in cerc spre dreapta
void rotRight() {
  Serial.println("rotireDreapta");
  digitalWrite(drFront, LOW);
  digitalWrite(stFront, HIGH);
  digitalWrite(drBack, HIGH);
  digitalWrite(stBack, LOW);
}
//functie care misca robotul in cerc spre stanga
void rotLeft() {
  Serial.println("rotireStanga");
  digitalWrite(drFront, HIGH);
  digitalWrite(stFront, LOW);
  digitalWrite(drBack, LOW);
  digitalWrite(stBack, HIGH);
}

//loop se apeleaza la infinit
void loop() {
  Usb.Task();
  
  if (inFata == 1 && inFataOld == 0) {
    forward();
    inFataOld = inFata;
  } else if (inFata == 0 && inFataOld == 1) {
    gata();
    inFataOld = inFata;
  }
  if (inSpate == 1 && inSpateOld == 0) {
    backwards();
    inSpateOld = inSpate;
  } else if (inSpate == 0 && inSpateOld == 1) {
    gata();
    inSpateOld = inSpate;
  }
  if (inStanga == 1 && inStangaOld == 0) {
    frontLeft();
    inStangaOld = inStanga;
  } else if (inStanga == 0 && inStangaOld == 1) {
    gata();
    inStangaOld = inStanga;
  }
  if (inDreapta == 1 && inDreaptaOld == 0) {
    frontRight();
    inDreaptaOld = inDreapta;
  } else if (inDreapta == 0 && inDreaptaOld == 1) {
    gata();
    inDreaptaOld = inDreapta;
  }
  if (rotStanga == 1 && rotStangaOld == 0) {
    rotLeft();
    rotStangaOld = rotStanga;
  } else if (rotStanga == 0 && rotStangaOld == 1) {
    gata();
    rotStangaOld = rotStanga;
  }
  if (rotDreapta == 1 && rotDreaptaOld == 0) {
    rotRight();
    rotDreaptaOld = rotDreapta;
  } else if (rotDreapta == 0 && rotDreaptaOld == 1) {
    gata();
    rotDreaptaOld = rotDreapta;
  }
  if (spateDreapta == 1 && spateDreaptaOld == 0) {
    backRight();
    spateDreaptaOld = spateDreapta;
  } else if (spateDreapta == 0 && spateDreaptaOld == 1) {
    gata();
    spateDreaptaOld = spateDreapta;
  }
  if (spateStanga == 1 && spateStangaOld == 0) {
    backLeft();
    spateStangaOld = spateStanga;
  } else if (spateStanga == 0 && spateStangaOld == 1) {
    gata();
    spateStangaOld = spateStanga;
  }
}
