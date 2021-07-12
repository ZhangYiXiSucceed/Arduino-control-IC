



### Arduino 控制RFID读写器读写 IC卡

#### 一、IC卡的认识

IC卡可以通过频段进行划分，<font color=red>**低频**</font>（125KHz~134kHz，典型工作频率是12KHz）、<font color=red>**高频**</font>（13.56MHz）和<font color=red>**超高频**</font>（860-960MHz）。

今天的主角是高频卡即咱的<font color=red>**M1卡**</font>，当然高频卡还有其他类型的卡，M1卡的芯片主要有s50和s70，即Philips Mifare 1 S50/S70芯片。区别是前者存储区只有1KB，后者4KB。  协议遵循<font color=red>**ISO 14443**</font> 和ISO 10536。



M1 S50卡共分为<font color=red>**16个扇区**</font>，每个扇区分为4个块，每个块<font color=red>**16个字节**</font>，所以总共16X4X16=1024字节，即1KB。



每个扇区的最后一个块，也就是块三，控制着整个扇区的数据读写权限，数据可进行增加和减少等等，具体显示如下。



| FF FF FF FF FF | FF 07 80 69 | FF FF FF FF FF |
| :------------: | :---------: | :------------: |
|     密码A      |  控制权限   |     密码B      |



控制字节的前三个字节为有效字节，最后一个字节为备用字节，通过位的组合来进行多种控制的选择，



比如第二个字节的第四位X、第三个字节的第一位Y和第三个字节的第四位Z组成一个0-7 的数据，每个数据代表一种控制方式，控制着**块0**的读写及数据加减操作。



|                 |  7   |  6   |  5   |   4   |  3   |  2   |  1   |   0   |
| --------------- | :--: | :--: | :--: | :---: | :--: | :--: | :--: | :---: |
| 字节1 （FF）    | Y3_b | Y2_b | Y1_b |  Y_b  | X3_b | X2_b | X1_b |  X_b  |
| 字节2  （07）   |  X3  |  X2  |  X1  | **X** | Z3_b | Z2_b | Z1_b |  Z_b  |
| 字节3 （80）    |  Z3  |  Z2  |  Z1  | **Z** |  Y3  |  Y2  |  Y1  | **Y** |
| 字节4（69备用） |      |      |      |       |      |      |      |       |

| 控制位 | 控制位 | 控制位 | 访问条件 | 访问条件 | 访问条件  | 访问条件 |
| :----: | :----: | :----: | :------: | :------: | :-------: | :------: |
|   X    |   Y    |   Z    |  读数据  |  写数据  | 数据增加+ | 数据减少 |
|   0    |   0    |   0    |  KEYA/B  |  KEYA/B  |  KEYA/B   |  KEYA/B  |
|   0    |   0    |   1    |  KEYA/B  |    X     |     X     |    X     |
|   0    |   1    |   0    |  KEYA/B  |   KEYB   |     X     |    X     |
|   0    |   1    |   1    |  KEYA/B  |   KEYB   |   KEYB    |  KEYA/B  |
|   1    |   0    |   0    |  KEYA/B  |    X     |     X     |  KEYA/B  |
|   1    |   0    |   1    |   KEYB   |   KEYB   |     X     |    X     |
|   1    |   1    |   0    |   KEYB   |    X     |     X     |    X     |
|   1    |   1    |   1    |    X     |    X     |     X     |    X     |

** <font color=red>**KEYA/B** </font>表示验证<font color=red>**AB密码均可** </font>，**X**表示没有该权限 <font color=red>**无法进行该操作**</font>。

X_b表示X的反码。

比如上面的控制权限 为000  即验证A或者B任意密码均可以进行读写加减数据等操作，属于安全性较低的一种控制权限。

然后<font color=red>**块1**</font>和<font color=red>**块2**</font>的读写权限也如上图所示，是由<font color=red>**X1** </font>，**<font color=red>Y1</font>**，**<font color=red>Z1</font>**和<font color=red>**X2**</font>、**<font color=red>Y2</font>**和**<font color=red>Z2</font>**来控制。



块3的读写权限，与块012有所不同。

| 控制位 | 控制位 | 控制位 | A密码访问条件 | A密码访问条件 | 存取控制访问条件 | 存取控制访问条件 | B密码访问条件 | B密码访问条件 |
| :----: | :----: | :----: | :-----------: | :-----------: | :--------------: | :--------------: | :-----------: | :-----------: |
|   X3   |   Y3   |   Z3   |    读数据     |    写数据     |      读数据      |      写数据      |    读数据     |    写数据     |
|   0    |   0    |   0    |       X       |    KEYA/B     |      KEYA/B      |        X         |    KEYA/B     |    KEYA/B     |
|   0    |   0    |   1    |       X       |       X       |      KEYA/B      |        X         |    KEYA/B     |       X       |
|   0    |   1    |   0    |       X       |     KEYB      |      KEYA/B      |        X         |       X       |     KEYB      |
|   0    |   1    |   1    |       X       |       X       |      KEYA/B      |        X         |       X       |       X       |
|   1    |   0    |   0    |       X       |    KEYA/B     |      KEYA/B      |      KEYA/B      |    KEYA/B     |    KEYA/B     |
|   1    |   0    |   1    |       X       |     KEYB      |      KEYA/B      |       KEYB       |       X       |     KEYB      |
|   1    |   1    |   0    |       X       |       X       |      KEYA/B      |       KEYB       |       X       |       X       |
|   1    |   1    |   1    |       X       |       X       |      KEYA/B      |        X         |       X       |       X       |



普通的M1白卡块0的前四位是<font color=red>**UID号**</font>，代表着该<font color=red>**卡唯一ID**</font>，<font color=red>**不可以随意修改**</font>，因此许多门禁卡将其作为<font color=red>**身份的识别**</font>，但是也有不少人想复制别人的卡，意思就是将自己卡的UID改成与别人的卡的UID一致。



普通的M1白卡无法修改，就出现了一种特殊的M1卡，<font color=red>**UID卡**</font>，也称为魔术卡，UID可以更改，但是不可以直接通过读写更改，需要<font color=red>**特殊的指令**</font>才可以修改，可以更改多次，但是<font color=red>**无法绕过读写器的防火墙**</font>，会响应读写器后指令，被发现为复制卡。



于是乎又出现了一种<font color=red>**CUID卡**</font>，可以直接通过<font color=red>**读写指令多次更改0扇区**</font>，方便了很多，同时会屏蔽后指令，<font color=red>**绕过防火墙**</font>，但是其ID可能被其他读写更改，可能导致失效。



于是乎又出现了一种<font color=red>**FUID卡**</font>，可<font color=red>**更改一次UID**</font>，之后将<font color=red>**变成普通的UID卡**</font>，防火墙检测也可以绕过，<font color=red>**只能读写一次**</font>。还有一种<font color=red>**UFUID卡**，如果<font color=red>**执行封卡操作**</font>，则变成<font color=red>**普通的M1卡**</font>，如果<font color=red>**不执行封卡操作**</font>，就是CUID卡，可以反复读写，是FUID和CUID的组合版，可能就是比较贵。



#### 二、Arduino读写IC卡取及UID号



笔者使用的Arduino套件中的RFID读写器，芯片采用NXP公司的RC522，SPI通信，由于Arduino中自带SPI和RFID的库，所以很方便的调用来初始化芯片，以及读取卡片信息。

读取IC卡的信息步骤：



<font color=blue>**0、初始化RC522**</font>

<font color=blue>**1、是否有新卡处于空闲状态，自动忽略睡眠的卡，会唤醒卡**</font>

<font color=blue>**2、发送选择指令选择 一张卡，**</font>

<font color=blue>**3、串口输出卡的存储转储文件信息。（dump文件）**</font>



```c
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

```



Arduino  读写IC卡内容的程序，主要是验证扇区AB密码、读块数据和写块数据，具体流程如下：



<font color=blue>**0、初始化RC522，定义要写入的数据等**</font>

<font color=blue>**1、察看是否有新卡**</font>

<font color=blue>**2、选择一张卡**</font>

<font color=blue>**3、显示卡片UID和类型**</font>

<font color=blue>**4、验证密钥，并读取整个扇区数据，具体验证A还是B密钥，需要知道卡的控制类型，根据上面知道的其实000，即验证AB密码均可。**</font>

<font color=blue>**5、验证秘钥，写入对应扇区对应块的数据。**</font>

<font color=blue>**6、检测写入的数据是否正确，先读出数据，然后和源数据做比较，**</font>



```C
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
  dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);    //将keybyte格式化成字符串，
  Serial.println();
  Serial.println(F("注意，会把数据写入到卡在#0"));
}


void loop() {
    
  MFRC522::StatusCode status;
  
  // 寻找新卡
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  // 选择一张卡
  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  // 显示卡片的详细信息
  Serial.print(F("卡片 UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);   //将uid格式化成字符串，
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

  // 我们使用第0个扇区
  // 修改块1
  byte sector         = 0;
  byte blockAddr      = 1;
  byte dataBlock[]    = {
    0x01, 0x02, 0x03, 0x04, //  1,  2,   3,  4,
    0x04, 0x06, 0x07, 0x08, //  5,  6,   7,  8,
    0x0A, 0x0B, 0x0C, 0x0D, //  0，0，0，0
    0x0E, 0x0F, 0x09, 0x10  // 0，0，0，0
  };//写入的数据定义
  byte trailerBlock   = 1;
 
  byte buffer[18];
  byte size = sizeof(buffer);

  // 原来的数据
  Serial.println(F("显示原本的数据..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("身份验证失败？或者是卡链接失败"));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // 显示整个扇区
  Serial.println(F("显示所有扇区的数据"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector);
  Serial.println();

  // 从块儿读取数据
  Serial.print(F("读取块儿的数据在：")); Serial.print(blockAddr);
  Serial.println(F("块 ..."));
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("读卡失败，没有连接上 "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.print(F("数据内容在第 ")); Serial.print(blockAddr); Serial.println(F(" 块:"));
  dump_byte_array(buffer, 16); Serial.println();
  Serial.println();

  //开始进行写入准备
  Serial.println(F("开始进行写入的准备..."));
  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("写入失败，没有连接上或者没有权限 "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return;
  }

  // Write data to the block
  Serial.print(F("在第： ")); Serial.print(blockAddr);
  Serial.println(F("  块中写入数据..."));
  dump_byte_array(dataBlock, 16); Serial.println();
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, dataBlock, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("写入失败... "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  Serial.println();


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
    if (buffer[i] == dataBlock[i])
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
```



Arduino 更改UID卡的ID号，UID卡的ID不能直接按常规的上面的方法写入，刚开始的时候，笔者并不知道如何更改UID号，就按照常规的方法写入，即验证秘钥A/B，然后写入，结果发现，每次都无法写入，提示写入失败，卡片无响应。



然后上网百度发现UID卡无法按照正常的方式写入，需要特殊的指令，这段指令和常规的写法不同，网上有实现采用STM32的，我采用Arduino的，都可以实现。



Sent bits: 50 00 57 cd       //休眠，50 00 就是 PcdHalt()
Sent bits: 40 (7 bits) （特殊指令）//第一条指令，要设定 BitFramingReg 使 传送 7位数据，不能是8位。
Received bits: a (4 bits)
Sent bits: 43 （特殊指令）//第二条指令
Received bits: 0a
Sent bits: a0 00 5f b1  //第三条指令
Received bits: 0a
Sent bits: 00 dc 44 20 b8 08 04 00 46 59 25 58 49 10 23 02 c0 10 //正常的写入块0数据
Received bits: 0a

经过查找和尝试，找到了库中的一个写入函数：PCD_TransceiveData，这个函数的参数挺复杂的，然后多次尝试之后，终于写成功了，



<font color=blue>**0、初始化RC522，定义要写入的数据等**</font>

<font color=blue>**1、察看是否有新卡**</font>

<font color=blue>**2、选择一张卡**</font>

<font color=blue>**3、显示卡片UID和类型**</font>

<font color=blue>**4、休眠**</font>

<font color=blue>**5、以此执行一二三条指令，返回正确则正常写入。**</font>

<font color=blue>**6、读取验证是否正常写入（未调试成功）**</font>

```c
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

  


  //开始进行写入准备
  Serial.println(F("开始进行写入的准备..."));


    status = mfrc522.PICC_HaltA();         //休眠
    if(status != MFRC522::STATUS_OK)
    {
      Serial.print(F("休眠失败... "));
      
      return;
    }
    
    Serial.println(F("休眠成功... "));
    
    //第一条指令、
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
    
    //第一条指令、
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

    //第三条指令、
    byte  data[20]={
    0x21, 0xB7, 0x7F, 0xE2, //  1,  2,   3,  4,
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
    data[4] = Value;
   mfrc522.PCD_CalculateCRC(data,16,&data[16]);
   status = mfrc522.PCD_TransceiveData(data,18,&receive,&rec_len,nullptr,0,false);
   if(status != MFRC522::STATUS_OK)
   {
       Serial.println(F("块0 发送失败... "));
       return;
   }
    Serial.println(receive); 


  byte sector         = 0; //0扇区
  byte blockAddr      = 0; //0块
 
  byte buffer[18]; 
  byte size = sizeof(buffer);
   

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
    if (buffer[i] == data[i])
      count++;
  }
  Serial.print(F("匹配的字节数量 = ")); Serial.println(count);
  if (count == 16) {
    Serial.println(F("验证成功!"));
  } else {
    Serial.println(F("失败，数据不匹配"));
    Serial.println(F("也许写入的内容不恰当"));
  }
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
```

> 修改UID卡学习即可，别做坏事呦！

