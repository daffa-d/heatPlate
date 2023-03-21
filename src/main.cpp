#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <Heat.h>
#include <SPI.h>
#include <DMD2.h>
#include <TimerOne.h>
#include <fonts/Arial_Black_16.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
heat plate(2, 0.0025, 9, A0);
SoftDMD dmd(3, 1);
DMD_TextBox box(dmd);

float setPoint = 0;
int set = 150;
int detik;
int mode = 0;
int modePlay = 0;
float temp;
bool state = false;

void lcdPrint(int x, int y, String text);
void time();
void clearLCDLine(int line);

const char *MESSAGE = " EEPROM FINISHHH !!!!! ";

// void ScanDMD()
// {
//   dmd.scanDisplayBySPI();
// }

void setup()
{
  Serial.begin(9600);
  plate.begin(3, 5, 50);
  pinMode(2, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(10, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  lcd.init();
  lcd.backlight();
  lcdPrint(2, 0, "Heater Plate");
  lcdPrint(5, 1, "Ready");
  delay(2000);
  lcd.clear();

  // Timer1.initialize( 5000 );           //period in microseconds to call ScanDMD. Anything longer than 5000 (5ms) and you can see flicker.
  // Timer1.attachInterrupt( ScanDMD );
  dmd.begin();
  dmd.setBrightness(75);
  dmd.selectFont(Arial_Black_16);
  dmd.clearScreen();
}

void loop()
{
  if (!digitalRead(10))
  {
    while (!digitalRead(10))
    {
    }
    modePlay++;
    clearLCDLine(1);
    if (modePlay > 2)
    {
      modePlay = 0;
    }
  }

  if (modePlay == 0)
  {
    lcd.clear();
    // dmd.clearScreen();
    // PANEL P10
    const char *next = MESSAGE;
    while (*next)
    {
      box.print(*next);
      next++;
      if (!digitalRead(10))
      {
        break;
      }
    }
  }

  else if (modePlay == 1)
  {
    dmd.clearScreen();
    time();
    plate.readTemp(temp);
    if (mode == 0)
    {
      dmd.drawString(0, 0, String(temp) + " C");
      lcdPrint(0, 0, "T: " + String(temp) + "C");
      lcdPrint(0, 1, "S: " + String(set));
      if (!digitalRead(2))
      {
        while (!digitalRead(2))
        {
        }
        set += 10;
        delay(10);
        if (set > 200)
        {
          set = 200;
        }
      }
      else if (!digitalRead(4))
      {
        while (!digitalRead(4))
        {
        }
        set -= 10;
        delay(10);
        if (set < 150)
        {
          set = 150;
        }
      }
      else if (!digitalRead(12))
      {
        while (!digitalRead(12))
        {
        }
        detik = 0;
        mode = 1;
        lcd.clear();
        delay(10);
      }
      if (detik > 2000)
      {
        detik = 0;
      }
    }

    else if (mode == 1)
    {
      dmd.drawString(0, 0, String(temp) + " C");
      lcdPrint(0, 0, "T: " + String(temp) + "C");
      lcdPrint(2, 1, "Preheat Dulu");
      setPoint = detik * 1.556; // 140
      if (temp > 140 && detik < 2500)
      {
        lcdPrint(1, 1, "Preheat Selesai");
        delay(500);
        lcd.clear();
        mode = 2;
      }
      state = true;
    }

    else if (mode == 2)
    {
      dmd.drawString(0, 0, String(temp) + " C");
      lcdPrint(0, 0, "T: " + String(temp) + "C");
      lcdPrint(0, 1, "Set to SetPoint");
      setPoint = set;
      if (temp > setPoint - 1 && temp < setPoint + 1)
      {
        lcdPrint(0, 1, "  SetPoint !!!  ");
        delay(500);
        detik = 0;
        mode = 3;
      }
      else
      {
        state = true;
      }
    }
    else if (mode == 3)
    {
      state = true;
      dmd.drawString(0, 0, String(temp) + " C");
      lcdPrint(0, 0, "T: " + String(temp) + "C");
      if (detik > 10 && detik < 50)
      {
        setPoint = set;
        if (temp > setPoint)
        {
          setPoint = 0;
        }
      }
      else if (detik > 55)
      {
        state = false;
        lcdPrint(0, 1, "Cooling        ");
        if (temp < 50)
        {
          lcdPrint(0, 1, "Cooling Selesai");
          delay(2000);
          lcd.clear();
          detik = 0;
          mode = 0;
        }
      }
    }
  }

  if (state)
  {
    plate.setPoint(setPoint);
  }
  else
  {
    analogWrite(3, 0);
    analogWrite(5, 0);
  }

  if (modePlay == 2)
  {
    time();
    plate.readTemp(temp);
    lcdPrint(0, 0, "T: " + String(temp) + "C");
    if(!digitalRead(2)){
      while(!digitalRead(2)){}
      lcdPrint(0, 1, "Manual Mode");
      mode += 1;
      if(mode > 2){
        mode = 0;
      }
    }
    if(mode == 1){
      plate.setPoint(180);
      lcdPrint(0, 1, "Panas BOSS");
    } else if(mode == 2){
      plate.setPoint(0);
      lcdPrint(0, 1, "Dingin BOSS");
    }

  }
}

void lcdPrint(int x, int y, String text)
{
  lcd.setCursor(x, y);
  lcd.print(text);
  lcd.print("    ");
}

void clearLCDLine(int line)
{
  for (int i = 0; i < 16; i++)
  {
    lcd.setCursor(i, line);
    lcd.print(" ");
  }
}

void time()
{
  unsigned long prevTime = 0;
  if (millis() - prevTime >= 1000)
  {
    prevTime = millis();
    detik++;
  }
}