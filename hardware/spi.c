#include "wizchip_conf.h"
#include "spi.h"


void W5500_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;	

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);

	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );
  
	// ???????SPI-CS?? (PA0)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
}

//以下是SPI模块的初始化代码，配置成主机模式，访问SD Card/W25Q64/NRF24L01						  
//SPI口初始化
//这里针是对SPI1的初始化
void SPI1_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOA, ENABLE );//PORTB时钟使能 
//	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_SPI1,  ENABLE );//SPI2时钟使能 	

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1 | RCC_APB2Periph_AFIO, ENABLE);
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  //复用推挽输出 
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOB

 	GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7);  //PB13/14/15上拉

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;		//设置SPI工作模式:设置为主SPI
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;   //设置SPI的数据大小:SPI发送接收8位帧结构
//	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;		    //时钟悬空低
//	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;	    //数据捕获于第1个时钟沿
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;		//串行同步时钟的空闲状态为高电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;	    //串行同步时钟的第二个跳变沿（上升或下降）数据被采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;		    //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;//波特率预分频值:波特率预分频值为256 初始化时为最低速模式
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	//指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
	SPI_InitStructure.SPI_CRCPolynomial = 7;	//CRC值计算的多项式
	SPI_Init(SPI1, &SPI_InitStructure); 
 
	SPI_Cmd(SPI1, ENABLE); //使能SPI外设
	
//	SPI1_ReadWriteByte(0xff);//启动传输		 
 

}   
//SPI 速度设置函数
//SpeedSet:
//SPI_BaudRatePrescaler_2   2分频   
//SPI_BaudRatePrescaler_8   8分频   
//SPI_BaudRatePrescaler_16  16分频  
//SPI_BaudRatePrescaler_256 256分频 
  
void SPI1_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI1->CR1&=0XFFC7;
	SPI1->CR1|=SPI_BaudRatePrescaler;	//设置SPI速度 
	SPI_Cmd(SPI1,ENABLE); 

} 

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
void SPI1_WriteByte(u8 TxData)
{		
	while((SPI1->SR&SPI_I2S_FLAG_TXE)==0);   //??????          
	SPI1->DR=TxData;                                         //????byte 
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==0); //???????byte  
	SPI1->DR;     
}


u8 SPI1_ReadByte()
{

	while((SPI1->SR&SPI_I2S_FLAG_TXE)==0);   //??????              
	SPI1->DR=0xFF;                                               //???????????????? 
	while((SPI1->SR&SPI_I2S_FLAG_RXNE)==0); //???????byte  
	return SPI1->DR;       
}



/**
  * @brief  ?????
  * @retval None
  */
void SPI_CrisEnter(void)
{
    __set_PRIMASK(1);
}
/**
  * @brief  ?????
  * @retval None
  */
void SPI_CrisExit(void)
{
    __set_PRIMASK(0);
}
 
/**
  * @brief  ?????????
  * @retval None
  */
void SPI_CS_Select(void)
{
    GPIO_ResetBits(GPIOA,GPIO_Pin_0);
}
/**
  * @brief  ?????????
  * @retval None
  */
void SPI_CS_Deselect(void)
{
    GPIO_SetBits(GPIOA,GPIO_Pin_0);
}
void register_wizchip()
{
	// First of all, Should register SPI callback functions implemented by user for accessing WIZCHIP 
	/* Critical section callback */
	reg_wizchip_cris_cbfunc(SPI_CrisEnter, SPI_CrisExit);   
	/* Chip selection call back */
	reg_wizchip_cs_cbfunc(SPI_CS_Select, SPI_CS_Deselect);
	
	/* SPI Read & Write callback function */
	reg_wizchip_spi_cbfunc(SPI1_ReadByte, SPI1_WriteByte);  
}



























