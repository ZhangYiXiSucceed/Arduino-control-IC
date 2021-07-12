/*
  【Arduino】108种传感器模块系列实验（资料+代码+图形+仿真）
  实验一百零二：MFRC-522 RFID射频 IC卡感应模块读卡器S50复旦卡钥匙扣
  1、工具-管理库-搜索“MFRC522”库-安装
  2、项目二：读取UID，并将其分别以十进制和十六进制输出到串口
  3、RFID与Arduino Uno的连线
  SDA------------------------Digital 10
  SCK------------------------Digital 13
  MOSI----------------------Digital 11
  MISO----------------------Digital 12
  IRQ------------------------不用连接
  GND-----------------------GND
  RST------------------------Digital 9
  3.3V------------------------3.3V (千万不要连接到5V接口！！！)
*/
#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <MFRC522Extended.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); //实例化类

// 初始化数组用于存储读取到的NUID
byte nuidPICC[4];

void setup() {
  Serial.begin(9600);
  SPI.begin(); // 初始化SPI总线
  rfid.PCD_Init(); // 初始化 MFRC522
}

void loop() {

  // 找卡
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // 验证NUID是否可读
  if ( ! rfid.PICC_ReadCardSerial())
    return;

   MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  // 检查是否MIFARE卡类型
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println("不支持读取此卡类型");
    return;
  }

  // 将NUID保存到nuidPICC数组
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte;
  }
  Serial.print("十六进制UID：");
  printHex(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  Serial.print("十进制UID：");
  printDec(rfid.uid.uidByte, rfid.uid.size);
  Serial.println();

  // 使放置在读卡区的IC卡进入休眠状态，不再重复读卡
  rfid.PICC_HaltA();

  // 停止读卡模块编码
  rfid.PCD_StopCrypto1();
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : "");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : "");
    Serial.print(buffer[i], DEC);
  }
}
