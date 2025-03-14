#include <LCD_I2C.h>

#define BTN_PIN 2
#define PHOTO_PIN A0
#define LED_PIN 8
#define JOYSTICK_PIN_X A1
#define JOYSTICK_PIN_Y A2

LCD_I2C lcd(0x27, 16, 2);
byte squid[8] = {B11111, B11111, B10101, B10101, B11111, B00100, B10101, B11111};
byte daEtudiant[8] = {B11100, B00100, B11100, B00101, B11101, B00001, B00001, B00001};
byte rightArrow[8] = {B00000, B11000, B11100, B11110, B11111, B11110, B11100, B11000};
byte leftArrow[8] = {B00000, B00011, B00111, B01111, B11111, B01111, B00111, B00011};
byte directionNull[8] = {B00100, B01110, B10101, B00100, B00100, B10101, B01110, B00100};


int speed_depart;
int speed_val = 0;
int direction_val = 0;
int ledState = 0;

unsigned long currentTime = 0;

void setup() {
  Serial.begin(115200);
  lcd.begin();

  lcd.createChar(0, squid);
  lcd.createChar(1, daEtudiant);
  lcd.createChar(2, rightArrow);
  lcd.createChar(3, leftArrow);
  lcd.createChar(4, directionNull);
  lcd.backlight();

  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);


  speed_depart = 1023 - analogRead(JOYSTICK_PIN_Y);
  Serial.print("Valeur de départ: ");
  Serial.println(speed_depart);
  setupLCD();
}

void loop() {
  currentTime = millis();

  speed_val = 1023 - analogRead(JOYSTICK_PIN_Y);
  direction_val = analogRead(JOYSTICK_PIN_X);

  affichageTask(currentTime, ledState, direction_val, speed_val);
}
int lightTask(unsigned long ct) {
  
  int photo_val = analogRead(PHOTO_PIN);
  int pourcentage = map(photo_val, 0, 1023, 0, 100);
  static unsigned long previousTime = 0;
  static int ledState = 0;
  

  lcd.setCursor(0, 0);
  lcd.print("Luminosite: ");
  lcd.print(pourcentage);
  lcd.print("% ");
  lcd.setCursor(0, 1);
  lcd.print("Phares: ");

  if((pourcentage < 50)) {
    temporisationTaskOn(ct, ledState, previousTime);
    if(!ledState) {
      lcd.setCursor(8, 1);
      lcd.print(((ct/1000) % 5) + 1);
      lcd.print("  ");
    }
    else {
      lcd.setCursor(8, 1);
      lcd.print("ON  ");
      
    }
    
  }
  else {
    temporisationTaskOff(ct, ledState, previousTime);
    if(ledState) {
      lcd.setCursor(8, 1);
      lcd.print(((ct/1000) % 5) + 1);
      lcd.print("  ");
    }
    else {
      lcd.setCursor(8, 1);
      lcd.print("OFF ");
    }

  }
  
  return ledState;
}
void temporisationTaskOn(unsigned long ct, int &ledState, unsigned long &previousTime) {
  const int rate = 5000;
  
    if(previousTime == 0) {
      previousTime = ct;
    }

    
    if(ct - previousTime >= rate) {
      if(!ledState) {
        ledState = 1;
      }
      else {
        previousTime = 0;
      }
      
    }
    
    
    digitalWrite(LED_PIN, ledState);

    
}
void temporisationTaskOff(unsigned long ct, int &ledState, unsigned long &previousTime) {
  const int rate = 5000;
  
    if(previousTime == 0) {
      previousTime = ct;
    }

    
    if(ct - previousTime >= rate) {
      if(ledState) {
        ledState = 0;
      }
      else {
        previousTime = 0;
      }
      
      
    }
    digitalWrite(LED_PIN, ledState);

    
}
void drivingTask(int &direction_val, int &speed_val) {
  
  int direction = map(direction_val, 0, 1023, -90, 90);
  
  int speed = 0;

  if (speed_val > speed_depart) {
    speed = map(speed_val, speed_depart, 1023, 0, 120);
  }
  else if(speed_val < speed_depart) {
    speed = map(speed_val, 0, speed_depart, -25, 0);
  }

  if(speed_val >= (speed_depart - 10) && speed_val <= (speed_depart + 10)) {
    speed = 0;
  }
  lcd.setCursor(0, 0);
  
  lcd.print("Vitesse: ");
  lcd.print(speed);
  if(speed < -10 || speed > 99) {
    lcd.print("km/h");
  }
  else if(speed >= 0 && speed < 10) {
    lcd.print("  km/h");
  }
  else {
    lcd.print(" km/h");
  }
  

  lcd.setCursor(0, 1);
  lcd.print("Direction: ");

  if(direction < 0) {
    lcd.write(byte(3));
  }
  else if(direction > 0) {
    lcd.write(byte(2));
  }
  else {
    lcd.write(byte(4));
  }

}
int estClic(unsigned long ct) {
  static int clic = 0;
  static int previous_state = 1;
  static unsigned long previousTime = 0;
  const int rate = 20;
  
  if(ct - previousTime < rate) {
    return clic;
  }

  previousTime = ct;

  int actual_state = digitalRead(BTN_PIN);
  
  if(actual_state == 0) {
    if(actual_state != previous_state) {
      clic = !clic;
    }
  }
  previous_state = actual_state;
  return clic;
}
void affichageTask(unsigned long ct, int ledState, int direction, int speed) {
  static unsigned long previousTime = 0;
  const int rate = 100;
  int phares;

  if(estClic(ct)) {
    drivingTask(direction, speed);
  }
  else {
    phares = lightTask(ct);
  }
  if(ct - previousTime >= rate) {
    Serial.print("etd:2066931, x:");
    Serial.print(direction_val);
    Serial.print(", y:");
    Serial.print(speed_val);
    Serial.print(", system:");
    Serial.println(phares);
  }
}
void setupLCD() {
  lcd.print("Beland");
  lcd.setCursor(0, 1);
  lcd.write(byte(0));
  lcd.setCursor(2, 1);
  lcd.write(byte(1));
  delay(3000);
  Serial.println("Je suis sensé écrire sur le LCD");
}