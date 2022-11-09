#include <Arduino.h>
#include <thermistor.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F, 16, 2);
thermistor therm(A0, 0);

float temp;
float PID_P, PID_I, PID_D;
float PID_ERROR, PREV_ERROR;
int setPoint = 0;
int set = 150;
float kP = 2, kI = 0.0025, kD = 9;
float PID_OUTPUT;
float PID_OUTPUT_MAX = 180;
float PID_OUTPUT_MIN = 0;
float pwmVal = 255;
int mode = 0;
int preheat = 70;
int detik;
int pTime, eTime;
float refreshRate = 500;
bool state = false;

void pidController();
void lcdPrint(int x, int y, String text);
void suhu();
void time();

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
  time();
  suhu();
  if (mode == 0)
  {
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    lcdPrint(0, 1, "S: " + String(set));
    if (!digitalRead(2))
    { 
      while(!digitalRead(2)){}
      set += 10;
      delay(10);
      if (set > 200)
      {
        set = 200;
      }
    }
    else if (!digitalRead(3))
    {
      while(!digitalRead(3)){}
      set -= 10;
      delay(10);
      if (set < 150)
      {
        set = 150;
      }
    }
    else if (!digitalRead(5))
    { 
      while(!digitalRead(5)){}
      detik = 0;
      mode = 1;
      lcd.clear();
      delay(10);
    }
    if(detik > 2000){
      detik = 0;
    }
  }

  else if (mode == 1)
  {
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    lcdPrint(2, 1, "Preheat Dulu");
    setPoint = detik * 1.556; // 140
    if (temp > 140 && detik < 2500)
    {
      lcdPrint(2, 1, "Preheat Selesai");
      delay(2000);
      lcd.clear();
      mode = 2;
    }
    state = true;
  }

  else if (mode == 2)
  {
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    lcdPrint(0, 1, "Set to SetPoint");
    setPoint = set;
    if (temp > setPoint - 2 && temp < setPoint + 2)
    {
        lcdPrint(2, 1, "SetPoint !!!");
        delay(2000);
        detik = 0;
        lcd.clear();
        mode = 3;  
    }
    else
    {
      state = true;
    }
  } else if (mode == 3 ){
    state = true;
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    lcdPrint(2,1, "Peak SetPoint");
    if(detik > 10 && detik < 1500) {
      setPoint = set; 
    } else if(detik > 1500){
        state = false;
        lcdPrint(2,1, "Cooling");
        if(temp < 50){
          lcdPrint(1,1, "Cooling Selesai");
          delay(2000);
          lcd.clear();
          detik = 0;
          mode = 0;
        }
    } 
    
  }

  if (state)
  {
    pidController();
  }
  else
  {
    analogWrite(10, 0);
    analogWrite(9, 0);
  }
}

void pidController()
{ 
  float PID_RefreshRate = 50;
  unsigned long prevTime_PID = 0;
  if(millis() - prevTime_PID >= PID_RefreshRate){
  PID_ERROR = setPoint - temp;
  PID_P = kP * PID_ERROR;
  PID_I = PID_I + (kI * PID_ERROR);
  PID_D = kD * (PID_ERROR - PREV_ERROR);
  PID_OUTPUT = PID_P + PID_I + PID_D;

  if (PID_OUTPUT > PID_OUTPUT_MAX)
  {
    PID_OUTPUT = PID_OUTPUT_MAX;
  }
  else if (PID_OUTPUT < PID_OUTPUT_MIN)
  {
    PID_OUTPUT = PID_OUTPUT_MIN;
  }
  pwmVal = 255 - PID_OUTPUT;
  // Serial.println(pwmVal);
  analogWrite(10, pwmVal);
  analogWrite(9, pwmVal);
  PREV_ERROR = PID_ERROR;
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

void time()
{
  unsigned long prevTime = 0;
  if (millis() - prevTime > 1000)
  {
    prevTime = millis();
    detik++;
  }
  Serial.println(detik);
}
