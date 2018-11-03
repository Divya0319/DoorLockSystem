#include <SPI.h>
#include <ShiftedLCD.h>
#include <SoftwareSerial.h>
#include <Keypad.h>
#include <EEPROM.h>
#include<Servo.h>
#define buzzer 6
#define greenLed A4
#define redLed A5

char enteredPassword[4];
int wrongAttempts = 0;
SoftwareSerial gsm(2, 9);
String outMsg = "Someone is trying to open your door.";
char destNum[10] = "7000493027", newDestNum[10] = "", prefix = "+91";
String strDestNum;
char in_password[4], new_password[4];
int i = 0;
Servo servo;
char globalKey = 0;
const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {3, 4, 5, 12};
byte colPins[COLS] = {A0, A1, A2, A3};
Keypad keypad4x4 = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(10);  //add latch pin number here (latch pin is where DS_14 pin from ic is connected to Arduino)

void setup() {
  lcd.begin(16, 2);
  gsm.begin(4800);
  pinMode(redLed, OUTPUT);
  pinMode(greenLed, OUTPUT);
  servo.attach(8);

  lockAtBegin();
  lcd.print("Enter Passkey:");
  lcd.setCursor(8, 1);
  lcd.print("#.R");
  lcd.setCursor(12, 1);
  lcd.print("*.N");
  lcd.setCursor(0, 1);
//    for (int j = 0; j < 4; j++)
//    {
//      EEPROM.write(j, j + 49);
//    }
  for (int j = 0; j < 10; j++)
    destNum[j] = EEPROM.read(j + 4);
  strDestNum = String(destNum);
  for (int j = 0; j < 4; j++)
    in_password[j] = EEPROM.read(j);
}

void loop() {
  globalKey = keypad4x4.getKey();
  if (globalKey == '#')
  {
    for (int i = 1500; i > 500; i--)
      tone(buzzer, i, 1);
    changePass();
  }
  if (globalKey == '*')
  {
    for (int i = 1500; i > 500; i--)
      tone(buzzer, i, 1);
    changeNum();
  }
  if (globalKey)
  {
    enteredPassword[i++] = globalKey;
    lcd.print("*");
    tone(buzzer, 1300, 50);
  }
  if (i == 4)
  {
    delay(200);
    for (int j = 0; j < 4; j++)
      in_password[j] = EEPROM.read(j);
    if (!(strncmp(enteredPassword, in_password, 4)))
    {
      tone(buzzer, 3000, 500);
      lcd.clear();
      lcd.print("Passkey Accepted");
      delay(2000);
      unLock();
      mPasskey();
    }
    else
    {
      tone(buzzer, 300, 1500);
      lcd.clear();
      lcd.print("Access Denied!!");
      delay(1000);
      wrongAttempts++;
      if (wrongAttempts % 3 == 0) {
        gsm.println("AT+CMGF=1"); // Set the Mode as Text Mode
        delay(1000);
        gsm.println("AT+CMGS=\"" + strDestNum + "\""); // Specify the Destination number in international format
        delay(1000);
        gsm.print(outMsg + " Your stored password is: " + in_password[0] + in_password[1] + in_password[2] + in_password[3]); // Enter the message
        gsm.write((char)26); // End of message character 0x1A : Equivalent to Ctrl+z
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Message sent.");
        delay(2000);
        lcd.clear();
      }
      mPasskey();
    }
    i = 0;
  }

}
void changePass()
{
  int j = 0;
  lcd.clear();
  lcd.print("Current Passkey:");
  lcd.setCursor(0, 1);
  while (j < 4)
  {
    char key = keypad4x4.getKey();
    if (key)
    {
      new_password[j++] = key;
      lcd.print("*");
      tone(buzzer, 1300, 50);
    }
    key = 0;
  }
  delay(500);
  if (strncmp(new_password, in_password, 4))
  {
    tone(buzzer, 500, 700);
    lcd.clear();
    lcd.print("Wrong Passkey!!");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(2000);
    changePass();
  }
  else
  {
    j = 0;
    lcd.clear();
    lcd.print("New Passkey:");
    lcd.setCursor(0, 1);
    while (j < 4)
    {
      char key = keypad4x4.getKey();

      if (key)
      {
        new_password[j++] = key;
        lcd.print(key);
        tone(buzzer, 1300, 50);

        if (j == 4)
        {
          if (strchr(new_password, '#') != NULL)
          {
            tone(buzzer, 200, 700);
            lcd.clear();
            lcd.print("Invalid Entry");
            delay(2000);
            key = 0;
            changePass();
            break;
          }
          else
          {
            for (int i = 0; i < 4; i++)
            {
              EEPROM.write(i, new_password[i]);
            }
            lcd.print(" Done......");
            delay(1000);
            for (int i = 0; i < 4; i++)
            {
              in_password[i] = EEPROM.read(i);
            }
            break;
          }
        }
      }
    }
  }
  mPasskey();
}
void changeNum()
{
  int j = 0;
  lcd.clear();
  lcd.print("Current Passkey:");
  lcd.setCursor(0, 1);
  while (j < 4)
  {
    char key = keypad4x4.getKey();
    if (key)
    {
      new_password[j++] = key;
      lcd.print("*");
      tone(buzzer, 1300, 50);
    }
    key = 0;
  }
  delay(500);
  if (strncmp(new_password, in_password, 4))
  {
    tone(buzzer, 500, 700);
    lcd.clear();
    lcd.print("Wrong Passkey!!");
    lcd.setCursor(0, 1);
    lcd.print("Try Again");
    delay(2000);
    changeNum();
  }
  else
  {
    j = 0;
    lcd.clear();
    lcd.print("New Phone No.:");
    lcd.setCursor(0, 1);
    while (j < 10)
    {
      char key = keypad4x4.getKey();

      if (key)
      {
        newDestNum[j++] = key;
        lcd.print(key);
        tone(buzzer, 1300, 50);

        if (j == 10)
        {
          if (strchr(newDestNum, '#') != NULL)
          {
            tone(buzzer, 200, 700);
            lcd.clear();
            lcd.print("Invalid Entry");
            delay(2000);
            key = 0;
            changeNum();
            break;
          }
          else
          {
            for (int i = 0; i < 10; i++)
            {
              EEPROM.write(i + 4, newDestNum[i]);
            }
            lcd.print(" Done.");
            delay(1000);
            for (int j = 0; j < 10; j++)
            {
              destNum[j] = EEPROM.read(j + 4);
            }
            strDestNum = String(destNum);
            break;
          }
        }
      }
    }
  }
  mPasskey();
}
void unLock() {
  digitalWrite(redLed, LOW); /** turns red LED off */
  digitalWrite(greenLed, HIGH); /** turns green LED on */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Door Unlocked"); /** Print 'Unlocked' */
  servo.write(0); /** rotate servo to its original position i.e unlock the door  */
  delay(1500); /** wait for 1.5 secs before triggering the loop **/
  for (int i = 15; i > 0; i--) /** This loop triggers a count-down timer of 15 seconds **/
  {
    if (i < 10) { /** if less than 10 seconds left on timer */
      tone(buzzer, 2000, 500);
      lcd.setCursor(0, 0);
      lcd.print("HURRY UP !!!!"); /** Print 'Hurry up' */
      lcd.setCursor(11, 1);
      lcd.print(" ");
      lcd.setCursor(12, 1);
      lcd.print(i); /** Print the seconds left */
    }
    else {
      lcd.setCursor(11, 1); /** Print i one position right, if its value is greater than 9 */
      lcd.print(i);
    }
    lcd.setCursor(0, 1);
    lcd.print("Locking in");
    lcd.setCursor(14, 1);
    lcd.print("..");
    delay(1000); /** i will decrement with a delay of 1 sec **/
  }
  lockAtBegin(); /** Lock the door **/
}
void lockAtBegin()
{
  digitalWrite(redLed, HIGH);
  digitalWrite(greenLed, LOW);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Door Locked");  /** Print 'Locked' */
  servo.write(90);  /** rotate servo by 90 degree i.e lock the door  */
  delay(2000);
  lcd.clear();
}
void mPasskey()
{
  lcd.clear();
  lcd.print("Enter Passkey:");
  lcd.setCursor(8, 1);
  lcd.print("#.R");
  lcd.setCursor(12, 1);
  lcd.print("*.N");
  globalKey = 0;
  lcd.setCursor(0, 1);
  if (globalKey)
  {
    lcd.print(globalKey);
  }
}

