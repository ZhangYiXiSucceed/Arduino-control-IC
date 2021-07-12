/*
【Arduino】108种传感器模块系列实验（资料+代码+图形+仿真）
实验一百零二：MFRC-522 RFID射频 IC卡感应模块读卡器S50复旦卡钥匙扣
1、工具-管理库-搜索“MFRC522”库-安装
2、项目：使用MFRC522 RFID和Arduino读写标签
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

#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 10
#define RST_PIN 9
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

void setup() {
  Serial.begin(9600); // Initialize serial communications with the PC
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  Serial.println("Scan PICC to see UID and type...");
}

void loop() {
  // Look for new cards


   byte  data[20]={
          
    0x80, 0xF2, 0xCA, 0x24, //  1,  2,   3,  4,
    0x9C, 0x08, 0x04, 0x00, //  5,  6,   7,  8,
    0x00, 0x00, 0x00, 0x00, //  0， 0，   0， 0
    0x00, 0x00, 0x00, 0x00  //  0， 0，   0， 0
   }; 
    byte i=0;
    byte Value = 0;
    for(i=0;i<4;i++)
    {
      Value^=data[i];
    }

//   Serial.print(Value, HEX);
   
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;
  }

  // Dump debug info about the card. PICC_HaltA() is automatically called.
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
}
