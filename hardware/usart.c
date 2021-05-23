#include "usart.h"


#if EN_USART1_RX   //如果使能了接收

int fputc(int ch, FILE *f)
{      
  while(USART_GetFlagStatus(USART1,USART_FLAG_TC)==RESET); 
    USART_SendData(USART1,(uint8_t)ch);   
  return ch;
}

//串口1中断服务程序
//注意,读取USARTx->SR能避免莫名其妙的错误     
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，  接收完成标志
//bit14，  接收到0x0d
//bit13~0，  接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记    

// Initialize UART1
void uart_init(u32 bound){
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1|RCC_APB2Periph_GPIOA, ENABLE);
  USART_DeInit(USART1);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // TX: PA9
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; // RX: PA10
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  USART_InitTypeDef USART_InitStructure;
  USART_InitStructure.USART_BaudRate = bound;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART1, &USART_InitStructure);

#if EN_USART1_RX
  NVIC_InitTypeDef NVIC_InitStructure;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
#endif
  USART_Cmd(USART1, ENABLE);
}

//这里 定义了一个16位数据USART_RX_STA 来表示 采集的数据长度  数据状态等 相当于一个寄存器
//USART_RX_STA     15        14           13-0
//         接收完成  接收到0x0d    接收的数据长度  没接收加1 表示多了一个字节
//USART_RX_STA=0 则为接收数据做准备

//串口进入中断的前提是 数据的最后以回车为准  即  0x0d 0x0a  
int usart_recv_flag = 0;
void USART1_IRQHandler(void)
{
  u8 Res;                          //当串口接收到数据  RXNE将被置1 
  if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
    Res = USART_ReceiveData(USART1); //(USART1->DR);  //读取接收到的数据
    if((USART_RX_STA&0x8000)==0) { //接收未完成
      if(USART_RX_STA&0x4000) {//接收到了0x0d
        if(Res!=0x0a)
          USART_RX_STA=0;//接收错误,重新开始
        else 
          USART_RX_STA|=0x8000;  //接收完成了         //接收到回车的后字节  置位状态寄存器 
        usart_recv_flag = 1;
        USART_RX_STA = 0;
      } else {//还没收到0X0D
        if(Res==0x0d)USART_RX_STA|=0x4000;           //接收到回车的前一字节  置位状态寄存器
        else {
          USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;      //将接收的数据 存入数组中
          USART_RX_STA++;                  //长度+1 为下一次做准备
          if(USART_RX_STA>(USART_REC_LEN-1))
            USART_RX_STA=0;//接收数据错误,重新开始接收    
        }
      }
    }        
  } 
}
#endif  















































