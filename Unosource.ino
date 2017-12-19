#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include "LedControlMS.h"

//Устанавливаем адрес 0x3f для дисплея и указываем его характеристики
LiquidCrystal_I2C lcd(0x3f, 16, 2);  

//Объект класса LedControl - управление матрицей светодиодов
//6 - DIN
//11 - CS
//12 - CLK
LedControl lc = LedControl(6,11,10);
//Пин, к которому подключен фоторезистор
int phResPin = 0; 
//Переменная, в которую считывется сопротивление фоторезистора
int phResVal;
//Пин, к которому подключен зуммер
int buzzerPin = 4;
//Значение, определяющее наименьшее сопротивление на фоторезисторе, при котором выключается подсветка дисплея и устанавливается минимальная яркость матрицы светодиодов
int minToneAndBr = 300;
String str = "";
//Количество обновлений - данных с ESP
int countOfUpd = 0;

void setup(){
  //инициализация дисплея
  lcd.init();
  lcd.backlight();
  Serial.begin(9600);
  lcd.clear();
  //инициализация работы с зуммером
  pinMode(buzzerPin, OUTPUT);
  //инициализация матричного дисплея
  //Включаем матрицу
  lc.shutdown(0,false);
  //Устанавливаем среднее значение яркости
  lc.setIntensity(0,8);
  //Очищаем дисплей
  lc.clearDisplay(0);
}

void loop(){
  //Если получены данные от ESP по поледовательному соединению
  if (Serial.available()) {
    countOfUpd++;
    delay(100);
    //Очищаем дисплей и возвращаемся в начальную позицию
    lcd.clear();
    //Считываем строку
    String str1;
    if (Serial.available()) {
      str1 = Serial.readStringUntil('\n');
    }
    //Добавляем пришедшую строку к имеющейся
    if(countOfUpd == 0)
      str = str1;
    else
      str += str1;
    lcd.print(str);
    //Считываем значение с фоторезистора
    phResVal = analogRead(phResPin);
    //Serial.println(phResVal);
    //Звуковое уведомление
    makeTone();
    //Изменяем яркость согласно сопротивлению фоторезистора
    changeBrightness();
  }
  char *g = new char[str.length() + 1];
  strcpy(g, str.c_str());
  if(countOfUpd > 0){
    delay(1000);
    //Сдвигаем дисплей
    scrollDisplay();
    //Выводим данные на матрицу
    lc.writeString(0,g);
    scrollDisplay();
  }
  delay(1000);
}

/**
 * Функция, сдвигающая дисплей влево
 */
void scrollDisplay(){
  lcd.scrollDisplayLeft();
}

/**
 * Функция выполняет вызов tone(), если значение сопротивления фоторезистора меньше, чем установленное
 */
void makeTone(){
  if(phResVal > minToneAndBr){
    tone(buzzerPin, phResVal, 5000);
  }
}

/**
 * Функция устанавливает минимальную яркость для дисплея и матрицы, если значение сопротивления фоторезистора меньше, чем установленное
 * В противном случае значения устанавливаются начальные
 */
void changeBrightness(){
  if(phResVal > minToneAndBr){
    lc.setIntensity(0,8);
    lcd.setBacklight(200);
  }
  else{
    lc.setIntensity(0,0);
    lcd.setBacklight(0);
  }
}
