/*如果RST管脚接MCU的通用IO脚,请参考下面的接口程序*/
/* MCU程序初试化时应设置IC_IO为输入,IC_IO最好设成open-drain模式。*/
/* 参考代码是基于51单片机的,其它型号的MCU需要正确设置IC_IO的输入/输出方向。*/
/*请按照参考代码中的提示位置进行设置，即在SendComm()函数开始第3行处，需要把MCU的IC_IO方向设为输出（最好设成open-drain开漏输出）；在SendComm()函数最后，需要把MCU的IC_IO方向设为输入；*/
/*在ResetCard()、BreakOperate()这两个函数开始，需要把MCU的IC_IO方向设为输入，否则会导致电平冲突而出错。MCU的IC_CLK、IC_RST方向一直是输出。*/
#include <stdio.h>
#include "autoconf_bt.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "internal_security.h"
#include "btdatatype.h"

#ifdef LICENSE_BY_CHIP

static unsigned char gpio_rst = 34;
static unsigned char gpio_clk = 35;
static unsigned char gpio_data = 36;

#define RST_GPIO_INDEX     gpio_rst
#define CLK_GPIO_INDEX     gpio_clk
#define DATA_GPIO_INDEX  	 gpio_data

// i8002 commands
#define RMM_COMM 0x30    //读主存命令字
#define WMM_COMM 0x38    //写主存命令字
#define VER_COMM 0x33    //校验密码
#define RSM_COMM 0x31    //读密码存储区
#define WSM_COMM 0x39    //写密码存储区
#define RPM_COMM 0x34    //读保护存储区
#define WPM_COMM 0x3c    //写保护存储区

//************************************************************************
//XZ8802底层程序的外部函数声明
void ReadMainMem(unsigned char addr,unsigned char  *pt,unsigned char count); //读XZ8802主存
void WriteMainMem(unsigned char addr,unsigned char  *pt);        //写XZ8802主存
void ReadProtectMem(unsigned char  *pt);                 //读保护存储器
void ProtectByte(unsigned char CardAdd,unsigned char  *pt);      //保护一字节,注意待保
//护的字节是已经写入过的,地址只能在保护存储区内
int Verify(unsigned char  *pt);                         //校验密码,成功返回1
void SendComm(unsigned char a,unsigned char b,unsigned char c);                      //发送命令
void ReadMode(unsigned char  *pt,unsigned char count);
void ProcessMode(void);
void ProcessMode_V(void);				//090615 //20101026
void ProcessMode_S(void);					 //20101026
void ResetCard(void);
void BreakOperate(void);
void StartComm(void);
void StopComm(void);
unsigned char ReadByte(void);
void WriteByte(unsigned char ch);
void Delay10us(void);
void Delay5us(void);

// RST, CLK and IO operation functions
void IC_RST_LOW(void);
void IC_RST_HIGH(void);
void IC_CLK_LOW(void);
void IC_CLK_HIGH(void);
void IC_IO_LOW(void);
void IC_IO_HIGH(void);
int	IO_PIN_State(void);
void Set_IO_IN(void);
void Set_IO_OUT(void);

static void EXT_GpioSetExport(int gpio);
static void EXT_GpioSetUnexport(int gpio);
static void EXT_GpioSetDirection(int gpio, int is_input);
static void EXT_GpioSetLevel(int gpio, int value);
static void EXT_GpioGetLevel(int gpio, int *lel);
/*********************************************************************

函 数 名: ReadMainMem()
功    能: 读XZ8802主存
说    明:
调    用:
全局变量:
入口参数: XZ8802地址(0 - 255),指向内部RAM的指针,字节数
出口参数：指向内部RAM的指针 *pt
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void ReadMainMem(unsigned char addr,unsigned char *pt,unsigned char count)
{
	/* //  ResetCard();
	BreakOperate();	/**/
	// NOTE: 需要进一步确认到底应该是用ResetCard还是BreakOperate，
	// 或者同时用。目前用6130, i163都测试通过。Jue，20110330
	ResetCard();   //[20140617LB]
	BreakOperate();
	SendComm(RMM_COMM,addr,0xff);	//第3个参数在此命令中被忽略，可以设成随机数
	ReadMode(pt,count);
	BreakOperate();
	// ResetCard();	//1212
}

/*********************************************************************

函 数 名: WriteMainMem()
功    能: 写XZ8802主存,一次 1Byte
说    明:
调    用:
全局变量:
入口参数: addr: XZ8802地址(32 - 255), pt: 指向数据区的指针  i:字节数
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void WriteMainMem(unsigned char addr,unsigned char  *pt)
{
	// ResetCard();
	SendComm(WMM_COMM, addr, *pt);     //写主存的命令字,地址,数据
	ProcessMode();
	// BreakOperate();
}
/*********************************************************************

函 数 名: ReadProtectMem()
功    能: 读保护存储器
说    明:
调    用:
全局变量:
入口参数: 指向直接寻址数据区的指针
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void ReadProtectMem(unsigned char  *pt)
{
	ResetCard();
	SendComm(RPM_COMM,0xff,0xff); //读保护存储器的命令字,后两个参数忽略，可以设成随机数
	ReadMode(pt,4);               //读出
	BreakOperate();
}
/*********************************************************************

函 数 名: ProtectByte()
功    能: 保护一字节,注意待保护的字节是已经写入过的,地址只能在保护存储区内
说    明:
调    用:
全局变量:
入口参数: XZ8802地址(0 ~ 31),指向直接寻址数据区的指针,pt指向的数据区应存放被保护的主存储器字节内容。
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void ProtectByte(unsigned char addr,unsigned char  *pt)
{
	ResetCard();
	SendComm(WPM_COMM, addr, *pt); //写保护存储区的命令字,地址,数据
	ProcessMode();
	BreakOperate();
}

/*********************************************************************

函 数 名: SendComm()
功    能: 发送命令
说    明:
调    用:
全局变量:
入口参数: a: 命令字, b: 地址, c: 数据
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void SendComm(unsigned char a,unsigned char b,unsigned char c)
{
	//PRINTMSGS(1, "[SendComm] start\r\n");

	//IC_CLK=0;	//0115
	IC_CLK_LOW();
	//test
	Delay5us();	//0115
	//如MCU不是51系列,须在此处设置MCU的IC_IO方向为输出!!（最好设成open-drain开漏输出）//0115 //0303  //090520
	// Set MCU IO5 OUT
	Set_IO_OUT();
	//test
	Delay5us();	//0115
	//PRINTMSGS(1, "[SendComm] to StartComm \r\n");
	StartComm();          //开始发送命令
	WriteByte(a);          //a: 发命令字
	WriteByte(b);          //b: 发地址
	WriteByte(c);          //c: 发数据
	//PRINTMSGS(1, "[SendComm] to StopComm\r\n");
	StopComm();           //结束发送命令
	//如MCU不是51系列,须在此处设置MCU的IC_IO方向为输入!!	//0115 //0218  //090520
	// Set MCU IO5 in
	Set_IO_IN();
//test
	Delay5us();	//0218


	//PRINTMSGS(1, "[SendComm] END\r\n");
}
/*********************************************************************

函 数 名: StartComm()
功    能: 开始命令模式
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void StartComm(void)
{
//    IC_CLK = 0;
//  IC_IO  = 1;
	IC_CLK_LOW();
	IC_IO_HIGH(); //IO pin
	Delay5us();
	//    IC_CLK = 1;
	IC_CLK_HIGH();
	Delay5us();
	//IC_IO  = 0;
	IC_IO_LOW();
	Delay5us();
	//PRINTMSGS(1, "[StartComm] End StartComm \r\n");
}
/*********************************************************************

函 数 名: StopComm()
功    能: 结束命令模式
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void StopComm(void)
{
//     IC_CLK = 0;
//     IC_IO  = 0;
	IC_CLK_LOW();
	IC_IO_LOW();
	Delay5us();
	//IC_CLK = 1;        //CLK: H
	IC_CLK_HIGH();
	Delay5us();
	//IC_IO  = 1;        //IO : H
	IC_IO_HIGH();
	Delay10us();
}
/*********************************************************************

函 数 名: ResetCard()
功    能: XZ8802复位和复位应答
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void ResetCard(void)
{
	unsigned char temp[4];
	unsigned char Temp[10]= {0};
//test
	Delay5us();
//	PRINTMSGS(1, "[ResetCard] to IC_RST_LOW \r\n");
	IC_RST_LOW();// = 0;
//	 PRINTMSGS(1, "[ResetCard] to IC_CLK_LOW \r\n");
	IC_CLK_LOW();// = 0;
	//如MCU不是51系列,须在此处设置MCU的IC_IO方向为输入!!	//0115  //090520
	//Set MCU IO5 IN
	Set_IO_IN();
//	PRINTMSGS(1, "[ResetCard] to IC_IO_HIGH \r\n");
	IC_IO_HIGH();//  = 1;
//test
	Delay5us();

//	PRINTMSGS(1, "[ResetCard] to IC_RST_HIGH \r\n");
	IC_RST_HIGH();// = 1;
//test
	Delay5us();
	IC_CLK_HIGH();// = 1;
	Delay10us();
	Delay10us();
	Delay10us();
	Delay10us();

	IC_CLK_LOW();// = 0;
//test
	Delay5us();
	IC_RST_LOW();// = 0;
//test
	Delay10us();     //复位和复位应答时序
	//      ___
	//RST _|   |_______________________
	//       _   _        __   __
	//CLK __|0|_|1|_ ..._|31|_|32|_____
	//   _   __  __           ___  _____
	//     \/  \/D0\ ...    \/D31\/
	//I/O _/\__/\__/        /\___/
	temp[0] = ReadByte();
	temp[1] = ReadByte();
	temp[2] = ReadByte();
	temp[3] = ReadByte();   //空读 32Bit (4Byte)
	IC_CLK_LOW();// = 0;         //     __   __  IC sets I/O to state H
//test
	Delay5us();         //CLK_|31|_|32|______________
	IC_IO_HIGH();//  = 1;          //     __   ___  ____________
	//_nop_();             //     30 \/ 31\/
	IC_CLK_LOW();// = 0;          //I/O  __ /\___/
//test
	Delay5us();
	//MessageBox(NULL,L"test2",L"ResetCard",MB_TOPMOST|MB_OK);
}
/*********************************************************************

函 数 名: BreakOperate()
功    能: XZ8802 操作中止
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: unsigned char
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
***********************************************************************/
void BreakOperate(void)
{
	IC_CLK_LOW();// = 0;
	//如MCU不是51系列,须在此处设置MCU的IC_IO方向为输入!!	//0115  //090520
	//t MCU IO5 IN
	Set_IO_IN();

	IC_RST_LOW();// = 0;
	IC_IO_LOW();//  = 0;
	Delay5us();
	IC_RST_HIGH();// = 1;
	IC_IO_HIGH();//  = 1;
	Delay5us();
	IC_RST_LOW();// = 0;
	Delay5us();
	IC_CLK_HIGH();// = 1;	//1212
	Delay5us();	//1212
	IC_CLK_LOW();// = 0;	//1212
	Delay5us();	//1212
	IC_CLK_HIGH();// = 1;	//1215
	Delay5us();	//1215
	IC_CLK_LOW();// = 0;	//1215
	Delay5us();	//1215
}
/*********************************************************************

函 数 名: ReadByte()
功    能: 从XZ8802读一个字节
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: unsigned char
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
***********************************************************************/
unsigned char ReadByte(void)
{
	unsigned char i,ch;
	ch = 0;
	for (i = 8; i > 0; i--) {
		IC_CLK_LOW();// = 0;
		Delay5us();		//edition 11
		ch = ch >> 1;           //从低位读起
		// if((unsigned char)IC_IO)   //IO pin
		if(IO_PIN_State())
			ch |= 0x80;
		//Delay5us();
		IC_CLK_HIGH();// = 1;
		Delay5us();
	}
	return ch;
}
/*********************************************************************

函 数 名: WriteByte()
功    能: 往XZ8802写一个字节
说    明: (内部函数)
调    用:
全局变量:
入口参数: unsigned char
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
***********************************************************************/
void WriteByte(unsigned char ch)
{
	unsigned char i;
	for(i = 8; i > 0; i--) {
		IC_CLK_LOW();// = 0;
		//IC_IO = (bit)(ch & 0x01);
		if( ch & 0x01) {
			IC_IO_HIGH();
		} else {
			IC_IO_LOW();
		}
		Delay5us();
		IC_CLK_HIGH();// = 1;
		Delay10us();
		ch = ch >> 1;                 //右移一位
	}
}
/*********************************************************************

函 数 名: ReadMode()
功    能: 连续输入i(=<255)个字节,存放到以pt开头的内部单元中,必须在某一读数据命令模式之后使用
说    明: (内部函数)
调    用:
全局变量:
入口参数: pt: 起始地址, count: 数据个数
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：
*********************************************************************/
void ReadMode(unsigned char  *pt,unsigned char count)
{
//test
	Delay5us();							//0217
	IC_CLK_LOW();//=0;
//test
	Delay5us();	//0115
	do {
		*pt = ReadByte();    //读入一个字节
		pt++;                //指针加一
	} while(--count);             //计数器减一,判断
	IC_CLK_LOW();// = 0;	//0115
}
/*********************************************************************

函 数 名: ProcessMode()
功    能: 处理模式,开始处理模式后，XZ8802将输入口拉低，处理完后输入口变成高电平
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                             日期：2006-08-10
*********************************************************************/
void ProcessMode(void)
{
	unsigned int i;
	Delay5us();							//0217
	IC_CLK_LOW();// = 0;
	Delay5us();  //0115
	IC_IO_LOW();//  = 0;
	for (i = 255; i > 0; i--) {
		IC_CLK_HIGH();// = 1;
		//Delay5us();
		Delay10us();
		IC_CLK_LOW();// = 0;
		//Delay5us();
		Delay10us();
	}
	//IC_IO  = 1;//2006-08-10
}

/*********************************************************************

函 数 名: ProcessMode_V()
功    能: 校验处理模式,开始校验处理模式后，XZ8802将输入口拉低，处理完后输入口变成高电平
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                         日期：2010-10-26
*********************************************************************/
void ProcessMode_V(void)	//20101026
{
	unsigned int i; //20101026
	//Delay5us();							//0217 //20101026
	IC_CLK_LOW();		//20101026
	//Delay5us();  //0115 //20101026
	IC_IO_LOW();		//20101026
	//for (i = 255; i > 0; i--) //20101026
	for (i = 8; i > 0; i--) { //20101026
		IC_CLK_HIGH();	//20101026
		//Delay5us();
		//Delay10us();  //20101026
		IC_CLK_LOW();	//20101026
		//Delay5us();
		//Delay10us();  //20101026
	}
	//IC_IO  = 1;//2006-08-10
}

/*********************************************************************

函 数 名: ProcessMode_S()
功    能: 写处理模式,开始写处理模式后，XZ8802将输入口拉低，处理完后输入口变成高电平
说    明: (内部函数)
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 无
设    计：Hentor                   日期：2005-03-18
修    改：                         日期：2010-10-26
*********************************************************************/
void ProcessMode_S(void)	//20101026
{
	unsigned int i;		  //20101026
	//Delay5us();							//0217 //20101026
	IC_CLK_LOW();		  //20101026
	//Delay5us();  //0115 //20101026
	IC_IO_LOW();		  //20101026
	//for (i = 255; i > 0; i--) //20101026
	for (i = 138; i > 0; i--) { //20101026
		IC_CLK_HIGH();	//20101026
		//Delay5us();
		//Delay10us();  //20101026
		IC_CLK_LOW();	//20101026
		//Delay5us();
		//Delay10us();  //20101026
	}
	//IC_IO  = 1;//2006-08-10
}


/********************************************************************

函 数 名：Delay10us()
功    能：延时10微秒（包括调用和返回的时间）,f = 100Mhz
说    明：请根据实际使用的CPU主频调整空指令循环常数
调    用：
入口参数：无
出口参数：无
返 回 值：无
设    计：Hentor           日    期：2005-03-18
修    改：                     日    期： 2006-08-10
***********************************************************************/
/*
void Delay10us(void)
{
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
    _nop_();
}
*/
//2006-08-10
void Delay10us(void)
{
	usleep(10);
//     unsigned  int  i;
//     PRINTMSGS(1,"=====\r\n");
//   	for(i=0;i<1000;i++) ;
// 	_nop_();
}
/********************************************************************

函 数 名: Delay5us()
功    能：延时5微秒（包括调用和返回的时间）,f = 100Mhz
说    明：请根据实际使用的CPU主频调整空指令循环常数
调    用：
入口参数：无
出口参数：无
返 回 值：无
设    计：Hentor           日    期：2005-03-18
修    改：                     日    期：2006-08-10
***********************************************************************/
/*
void Delay5us(void)
{
    _nop_();
    _nop_();
    _nop_();
}
*/
//2006-08-10
void Delay5us(void)
{
	usleep(5);
//    unsigned  int  i;
//  	for(i=0;i<500;i++)  ;
//	_nop_();
}
/*//////////////////////////////////////////////////////////////////////*/
/********************************************************************

// 调用Verify()函数校验旧密码通过后，写入新密码可参考如下代码：
// (校验旧密码如不通过，则不能写入新密码)
***********************************************************************/
// ResetCard();
// SendComm(WSM_COMM, 1, 0x1111);//PassWord_1); //修改密码字节1	//0115
// ProcessMode();
// SendComm(WSM_COMM, 2, 0x1111);//PassWord_2); //修改密码字节2
// ProcessMode();
// SendComm(WSM_COMM, 3, 0x1111);//PassWord_3); //修改密码字节3
// ProcessMode();

/********************************************************************
//请在调用完XZ8802的相关接口函数后增加如下几条语句：//090520
*********************************************************************/
// ResetCard();										//090520
// SendComm(WMM_COMM, 0x00, 0xff);     //写主存的0x00地址处为0xff,请不要改写成其他值。	//090520
// ProcessMode();										//090520
// ResetCard();										//090520

void IC_RST_LOW(void) //t pin RST low
{
	EXT_GpioSetLevel(RST_GPIO_INDEX, 0);
}

void IC_RST_HIGH(void)//t pin RST high
{
	EXT_GpioSetLevel(RST_GPIO_INDEX, 1);
}

void IC_CLK_LOW(void)
{
	EXT_GpioSetLevel(CLK_GPIO_INDEX, 0);
}

void IC_CLK_HIGH(void)
{
	EXT_GpioSetLevel(CLK_GPIO_INDEX, 1);
}

void IC_IO_LOW(void)
{
	EXT_GpioSetLevel(DATA_GPIO_INDEX, 0);
}

void IC_IO_HIGH(void)
{
	EXT_GpioSetLevel(DATA_GPIO_INDEX, 1);
}

int IO_PIN_State(void)
{
	unsigned char lel = 0;
	EXT_GpioGetLevel(DATA_GPIO_INDEX, &lel);

	if (lel) {
		return 1;
	} else {
		return 0;
	}
}

void Set_IO_OUT(void)
{
	EXT_GpioSetDirection(DATA_GPIO_INDEX, 1);
}

void Set_IO_IN(void)
{
	EXT_GpioSetDirection(DATA_GPIO_INDEX, 0);
}

//20140616New
/*********************************************************************

函 数 名: Verify_0(void)
功    能: 校验
说    明:
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 成功返回1,失败返回0
设    计：Hentor                   日期：2013-12-18
修    改：                         日期：
*********************************************************************/

unsigned char Verify_0(void)
{
	unsigned char temp[4];
	unsigned char i;
	ResetCard();
	SendComm(0x86,0xff,0xff);
	ReadMode(temp, 4);
	if((temp[2]== 0xff)&&(temp[3]== 0xff))
		return 1 ;
	else
		return 0;
}

/*********************************************************************

函 数 名: Verify_1(void)
功    能: 校验
说    明:
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 成功返回1,失败返回0
设    计：Hentor                   日期：2013-12-18
修    改：                         日期：
*********************************************************************/

unsigned char Verify_1(void)
{
	unsigned char temp[4];
	char hint[127] = {0};
	unsigned char i;
	ResetCard();
	SendComm(RSM_COMM,0xff,0xff);
	ReadMode(temp, 4);

	sprintf(hint, "Verify_1: %02x,%02x,%02x,%02x\r\n", temp[0], temp[1], temp[2], temp[3]);

	//if((temp[0]== 0)&& (temp[1]== 0)&&(temp[2]== 0)&&(temp[3]== 0)),  (bug : temp[0] laways 0xff)
	if((temp[1]== 0)&&(temp[2]== 0)&&(temp[3]== 0))
		return 1 ;
	else    return 0;
}

/*********************************************************************

函 数 名: Verify_2(void)
功    能: 校验
说    明:
调    用:
全局变量:
入口参数: 无
出口参数：无
返 回 值: 成功返回1,失败返回0
设    计：Hentor                   日期：2013-12-18
修    改：                         日期：
*********************************************************************/

unsigned char Verify_2(void)
{
	unsigned char temp[4];
	unsigned char i;
	ResetCard();
	SendComm(0x98,0xff,0xff);
	ReadMode(temp, 4);
	if((temp[2]== 0xff)&&(temp[3]== 0xff))
		return 1 ;
	else    return 0;
}

unsigned char VerifyTimeQue(unsigned char rst, unsigned char clk, unsigned char data)
{
	unsigned char bCheckID = 0;

	RST_GPIO_INDEX = rst;
	CLK_GPIO_INDEX = clk;
	DATA_GPIO_INDEX = data;

	EXT_GpioSetExport(RST_GPIO_INDEX);
	EXT_GpioSetExport(CLK_GPIO_INDEX);
	EXT_GpioSetExport(DATA_GPIO_INDEX);

	EXT_GpioSetDirection(RST_GPIO_INDEX, 1);
	EXT_GpioSetDirection(CLK_GPIO_INDEX, 1);
	EXT_GpioSetDirection(DATA_GPIO_INDEX, 1);

	bCheckID = Verify_0();
	if (bCheckID) {
		bCheckID = Verify_1();
		if (bCheckID) {
			bCheckID = Verify_2();
		}
	}

	EXT_GpioSetUnexport(RST_GPIO_INDEX);
	EXT_GpioSetUnexport(CLK_GPIO_INDEX);
	EXT_GpioSetUnexport(DATA_GPIO_INDEX);

	return bCheckID;
}

static BOOL g_verify_result = TRUE;
void set_verify(BOOL value)
{
	g_verify_result = value;
}
BOOL get_verify()
{
	return g_verify_result;
}

static int set_gpio_value(char *gpio_path, char* val)
{
	int size = 0;
	int fd = -1;
	int ret = -1;
	size = strlen(val);
	fd = open(gpio_path, O_WRONLY);
	if(fd < 0) {
		//
		return -1;
	}
	size = write(fd, val, size);
	if (size < 0) {
		//
	} else {
		ret = 0;
	}
	if (fd >= 0) {
		close(fd);
	}
	return ret;
}

static void EXT_GpioSetExport(int gpio)
{
	char gpio_path[128] = {0};
	sprintf(gpio_path, "/sys/class/gpio/export");
	char buffer[10] = {0};
	sprintf(buffer, "%d", gpio);
	int ret = set_gpio_value(gpio_path, buffer);
}

//void set_gpio_unexport(int gpio)
static void EXT_GpioSetUnexport(int gpio)
{
	char gpio_path[128] = {0};
	sprintf(gpio_path, "/sys/class/gpio/unexport");
	char buffer[10] = {0};
	sprintf(buffer, "%d", gpio);
	int ret = set_gpio_value(gpio_path, buffer);
}

static void EXT_GpioSetDirection(int gpio, int is_input)
{
	char gpio_path[128] = {0};
	sprintf(gpio_path, "/sys/class/gpio/gpio%d/direction", gpio);
	char buffer[10] = {0};
	sprintf(buffer, "%s", is_input?"out":"in");
	int ret = set_gpio_value(gpio_path, buffer);
}

static void EXT_GpioSetLevel(int gpio, int value)
{
	char gpio_path[128] = {0};
	sprintf(gpio_path, "/sys/class/gpio/gpio%d/value", gpio);
	char buffer[10] = {0};
	sprintf(buffer, "%d", value);
	int ret = set_gpio_value(gpio_path, buffer);
}


static void EXT_GpioGetLevel(int gpio, int *lel)
{
	int gpio_fd;
	char path_buf[128] = {0};
	char buff[10] = {0};
	sprintf(path_buf, "/sys/class/gpio/gpio%d/value", gpio);
	gpio_fd = open(path_buf, O_RDONLY);
	if( gpio_fd == -1 )
		printf("gpio open failed");
	int ret = read(gpio_fd,buff,10);
	*lel = atoi(buff);

	close(gpio_fd);
}

#endif
