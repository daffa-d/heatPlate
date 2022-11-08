#include <Arduino.h>
#include <thermistor.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
thermistor therm(A0, 0);

float temp;
float PID_P, PID_I, PID_D;
float PID_ERROR, PREV_ERROR;
int setPoint = 0;
int set = 120;
float kP = 0.1, kI = 0.01, kD = 0.01;
float PID_OUTPUT;
float pwmVal = 255;
int mode = 0;
int preheat = 70;
int detik;

void pidController();
void lcdPrint(int x, int y, String text);
void suhu();

void setup()
{
  Serial.begin(9600);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);
  for (int i = 2; i < 6; i++)
  {
    pinMode(i, INPUT_PULLUP);
  }
  lcd.init();
  lcd.backlight();
  lcdPrint(2, 0, "Heater Plate");
  lcdPrint(5, 1, "Ready");
  delay(2000);
  lcd.clear();
}

void loop()
{
  suhu();
  if (mode == 0)
  {
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    lcdPrint(0, 1, "S: " + String(set));
    if (!digitalRead(2))
    {
      set += 10;
      delay(10);
      if (set > 300)
      {
        set = 300;
      }
    }
    else if (!digitalRead(3))
    {
      set -= 10;
      delay(10);
      if (set < 120)
      {
        set = 120;
      }
    }
    else if (!digitalRead(5))
    {
      mode = 1;
      lcd.clear();
      delay(10);
    }
  }

  else if (mode == 1)
  {
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    lcdPrint(2, 1, "Preheat Dulu");
    setPoint = 200;
    if (temp > 100 && temp < 120)
    {
      lcdPrint(2, 1, "Preheat Selesai");
      delay(3000);
      lcd.clear();
      mode = 2;
    }
  }

  else if (mode == 2)
  {
    lcdPrint(0,0, "T: " + String(temp) + "C");
    lcdPrint(0,1, "Set to SetPoint");
    setPoint = set;
    if(temp > setPoint - 5 && temp < setPoint + 5){
      lcdPrint(2,1, "SetPoint Reached");
      delay(3000);
      lcd.clear();
      mode = 3;
    }
  }
  pidController();
}

void pidController()
{
  if (mode == 0)
  {
    setPoint = 255;
  } else { 
    PID_ERROR = setPoint - temp;
    PID_P = kP * PID_ERROR;
    PID_I = PID_I + (kI * PID_ERROR);
    PID_D = kD * (PID_ERROR - PREV_ERROR);
    PID_OUTPUT = PID_P + PID_I + PID_D;
    pwmVal = 255 - PID_OUTPUT;

    if (pwmVal <= 0)
    {
      pwmVal = 0;
    }
    else if (pwmVal >= 255)
    {
      pwmVal = 255;
    }
    Serial.println(pwmVal);
    analogWrite(10, pwmVal);
    analogWrite(9, pwmVal);
    PREV_ERROR = PID_ERROR;
    delay(10);
  }
}

void lcdPrint(int x, int y, String text)
{
  lcd.setCursor(x, y);
  lcd.print(text);
  lcd.print(" ");
}

void suhu()
{
  unsigned long prevTime = 0;
  if (millis() - prevTime >= 100)
  {
    temp = therm.analog2temp();
  }
}
