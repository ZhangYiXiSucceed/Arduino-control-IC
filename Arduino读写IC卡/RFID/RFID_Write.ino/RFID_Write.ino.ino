/*
  【Arduino】108种传感器模块系列实验（资料+代码+图形+仿真）
  实验一百零二：MFRC-522 RFID射频 IC卡感应模块读卡器S50复旦卡钥匙扣
  1、安装库：IDE-工具-管理库-搜索“MFRC522”库-安装
  2、项目三：RC522 模块的读写操作
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
#define RST_PIN         9           // 配置针脚
#define SS_PIN          10
MFRC522 mfrc522(SS_PIN, RST_PIN);   // 创建新的RFID实例
MFRC522::MIFARE_Key key;
void setup() {
  Serial.begin(9600); // 设置串口波特率为9600
  while (!Serial);    // 如果串口没有打开，则死循环下去不进行下面的操作
  SPI.begin();        // SPI开始
  mfrc522.PCD_Init(); // Init MFRC522 card

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("扫描卡开始进行读或者写"));
  Serial.print(F("使用A和B作为键"));
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();

  Serial.println(F("注意，会把数据写入到卡在#0"));
}


void loop() {
    
    MFRC522::StatusCode status;
 
//    byte buffer_send_Temp[4]={0x26,0x00,0x00,0x00};
//    byte receive_Temp[4];
//    byte rec_len_Temp = 2;
//
//    mfrc522.PCD_WriteRegister(MFRC522::TxModeReg, 0x00);
//    mfrc522.PCD_WriteRegister(MFRC522::RxModeReg, 0x00);
//    // Reset ModWidthReg
//    mfrc522.PCD_WriteRegister(MFRC522::ModWidthReg, 0x26);
//  
//    //mfrc522.PCD_SetRegisterBitMask(MFRC522::BitFramingReg,0x07);
//    status = mfrc522.PICC_RequestA(receive_Temp,&rec_len_Temp);
//   if(status != MFRC522::STATUS_OK)
//   {
//      Serial.println(F("0x40 发送失败... "));
//      Serial.println(status); 
//      return;
//   }
//    Serial.println(receive_Temp[0],HEX);
//     Serial.println(receive_Temp[1],HEX);
//    
//     Serial.println(F("寻卡成功 "));
//     return;
  
  // 寻找新卡
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // 选择一张卡
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  // 显示卡片的详细信息
  Serial.print(F("卡片 UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("卡片类型: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // 检查兼容性
  if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
          &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("仅仅适合Mifare Classic卡的读写"));
    return;
  }

  // 我们只使用第二个扇区
  // 覆盖扇区4
  byte sector         = 0;
  byte blockAddr      = 0;
  byte dataBlock[]    = {
    0x00, 0x00, 0x00, 0x00, //  1,  2,   3,  4,
    0x00, 0x00, 0x00, 0x00, //  5,  6,   7,  8,
    0x00, 0x00, 0x00, 0x00, //  0，0，0，0
    0x00, 0x00, 0x00, 0x00  // 0，0，0，0
  };//写入的数据定义
  byte trailerBlock   = 3;
 
  byte buffer[18];
  byte size = sizeof(buffer);

  // 原来的数据
    Serial.println(F("显示原本的数据..."));
//  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("身份验证失败？或者是卡链接失败"));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return;
//  }
//
//  // 显示整个扇区
//  Serial.println(F("显示所有扇区的数据"));
//  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
//  Serial.println();
//
//  // 从块儿读取数据
//  Serial.print(F("读取块儿的数据在：")); Serial.print(blockAddr);
//  Serial.println(F("块 ..."));
//  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("读卡失败，没有连接上 "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//  }
//  Serial.print(F("数据内容在第 ")); Serial.print(blockAddr); Serial.println(F(" 块:"));
//  dump_byte_array(buffer, 16); Serial.println();
//  Serial.println();

  //开始进行写入准备
  Serial.println(F("开始进行写入的准备..."));
//  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("写入失败，没有连接上或者没有权限 "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return;
//  }

//  // Write data to the block
//  Serial.print(F("在第： ")); Serial.print(blockAddr);
//  Serial.println(F("  块中写入数据..."));
//  dump_byte_array(dataBlock, 16); Serial.println();
//  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
//  if (status != MFRC522::STATUS_OK) {
//    Serial.print(F("写入失败... "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//  }
//  Serial.println();

    status = mfrc522.PICC_HaltA();         //休眠
    if(status != MFRC522::STATUS_OK)
    {
      Serial.print(F("休眠失败... "));
      
      return;
    }
    
    Serial.println(F("休眠成功... "));

      
    byte buffer_send[4]={0x40,0x00,0x00,0x00};
    
    byte receive= 0x01;
    byte rec_len = 1;
    byte validbits = 0x07;
    mfrc522.PCD_SetRegisterBitMask(MFRC522::BitFramingReg,validbits);
    status = mfrc522.PCD_TransceiveData(buffer_send,1,&receive,&rec_len,&validbits,0,false);
   if(status != MFRC522::STATUS_OK)
   {
      Serial.println(F("0x40 发送失败... "));
      Serial.println(status); 
      return;
   }
     
    Serial.println(receive); 

    buffer_send[0] = 0x43;
    validbits = 8;
    mfrc522.PCD_ClearRegisterBitMask(MFRC522::BitFramingReg,0x07);
     status = mfrc522.PCD_TransceiveData(buffer_send,1,&receive,&rec_len,nullptr,0,false);
   if(status != MFRC522::STATUS_OK)
   {
      Serial.println(F("0x43 发送失败... "));
      return;
   } 
     
    Serial.println(receive); 


    buffer_send[0] = 0xa0;
    buffer_send[1] = 0x00;
    buffer_send[2] = 0x5f;
    buffer_send[3] = 0xb1;
    
    validbits = 8;
    
     status = mfrc522.PCD_TransceiveData(buffer_send,4,&receive,&rec_len,nullptr,0,false);
   if(status != MFRC522::STATUS_OK)
   {
      Serial.println(F("块0 发送失败... "));
      return;
   } 
      
    Serial.println(receive); 


    byte  data[20]={
          
    0x30, 0x7E, 0xD5, 0x24, //  1,  2,   3,  4,
    0xBF, 0x08, 0x04, 0x00, //  5,  6,   7,  8,
    0x00, 0x00, 0x00, 0x00, //  0， 0，   0， 0
    0x00, 0x00, 0x00, 0x00  //  0， 0，   0， 0
   }; 


   mfrc522.PCD_CalculateCRC(data,16,&data[16]);
   status = mfrc522.PCD_TransceiveData(data,18,&receive,&rec_len,nullptr,0,false);
   if(status != MFRC522::STATUS_OK)
     {
      Serial.println(F("块0 发送失败... "));
       return;
     }
    Serial.println(receive); 
   

  // 再次读取卡中数据，这次是写入之后的数据
  Serial.print(F("读取写入后第")); Serial.print(blockAddr);
  Serial.println(F(" 块的数据 ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("读取失败... "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("块 ")); Serial.print(blockAddr); Serial.println(F("数据为 :"));
  dump_byte_array(buffer, 16); Serial.println();

  // 验证一下数据，要保证写入前后数据是相等的
  // 通过计算块中的字节数量
  Serial.println(F("等待验证结果..."));
  byte count = 0;
  for (byte i = 0; i < 16; i++) {
    // 比较一下缓存中的数据（我们读出来的数据） = （我们刚刚写的数据）
    if (buffer == dataBlock)
      count++;
  }
  Serial.print(F("匹配的字节数量 = ")); Serial.println(count);
  if (count == 16) {
    Serial.println(F("验证成功 :"));
  } else {
    Serial.println(F("失败，数据不匹配"));
    Serial.println(F("也许写入的内容不恰当"));
  }
  Serial.println();

  // 转储扇区数据
  Serial.println(F("写入后的数据内容为：:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // 停止 PICC
  mfrc522.PICC_HaltA();
  //停止加密PCD
  mfrc522.PCD_StopCrypto1();
}

/**
  将字节数组转储为串行的十六进制值
*/
void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
