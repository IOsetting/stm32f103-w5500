#include "delay.h"
#include "led.h"
#include "usart.h"
#include "spi.h"
#include "string.h"
#include "socket.h"
#include "dhcp.h"

/* DATA_BUF_SIZE define for Loopback example */
#ifndef DATA_BUF_SIZE
	#define DATA_BUF_SIZE			2048
#endif

/************************/
/* Select LOOPBACK_MODE */
/************************/
#define LOOPBACK_MAIN_NOBLOCK   0
#define LOOPBACK_MODE           LOOPBACK_MAIN_NOBLOCK

//u32 W5500_Send_Delay=0; //W5500发送延时计数变量(ms)
wiz_NetInfo gWIZNETINFO;

void Load_Net_Parameters(void)
{
	gWIZNETINFO.gw[0] = 192; //Gateway
	gWIZNETINFO.gw[1] = 168;
	gWIZNETINFO.gw[2] = 6;
	gWIZNETINFO.gw[3] = 1;

	gWIZNETINFO.sn[0]=255; //Mask
	gWIZNETINFO.sn[1]=255;
	gWIZNETINFO.sn[2]=255;
	gWIZNETINFO.sn[3]=0;

	gWIZNETINFO.mac[0]=0x0c; //MAC
	gWIZNETINFO.mac[1]=0x29;
	gWIZNETINFO.mac[2]=0xab;
	gWIZNETINFO.mac[3]=0x7c;
	gWIZNETINFO.mac[4]=0x00;
	gWIZNETINFO.mac[5]=0x01;

	gWIZNETINFO.ip[0]=192; //IP
	gWIZNETINFO.ip[1]=168;
	gWIZNETINFO.ip[2]=6;
	gWIZNETINFO.ip[3]=204;

	gWIZNETINFO.dhcp = NETINFO_STATIC;
}
uint8_t destip[4] = {192, 168, 6, 210};
uint16_t destport = 3333;
uint8_t buf[DATA_BUF_SIZE];

int32_t loopback_tcpc(uint8_t sn, uint8_t* buf, uint8_t* destip, uint16_t destport)
{
  int32_t ret; // return value for SOCK_ERRORs
  uint16_t size = 0, sentsize=0;

  // Destination (TCP Server) IP info (will be connected)
  // >> loopback_tcpc() function parameter
  // >> Ex)
  //	uint8_t destip[4] = 	{192, 168, 0, 214};
  //	uint16_t destport = 	5000;

  // Port number for TCP client (will be increased)
  static uint16_t any_port = 	50000;

  // Socket Status Transitions
  // Check the W5500 Socket n status register (Sn_SR, The 'Sn_SR' controlled by Sn_CR command or Packet send/recv status)
  switch(getSn_SR(sn))
  {
    case SOCK_ESTABLISHED :
      if(getSn_IR(sn) & Sn_IR_CON) { // Socket n interrupt register mask; TCP CON interrupt = connection with peer is successful
#ifdef _LOOPBACK_DEBUG_
        printf("%d:Connected to - %d.%d.%d.%d : %d\r\n",sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
        setSn_IR(sn, Sn_IR_CON);  // this interrupt should be write the bit cleared to '1'
      }

      //////////////////////////////////////////////////////////////////////////////////////////////
      // Data Transaction Parts; Handle the [data receive and send] process
      //////////////////////////////////////////////////////////////////////////////////////////////
      if((size = getSn_RX_RSR(sn)) > 0) { // Sn_RX_RSR: Socket n Received Size Register, Receiving data length
        if(size > DATA_BUF_SIZE) size = DATA_BUF_SIZE; // DATA_BUF_SIZE means user defined buffer size (array)
        ret = recv(sn, buf, size); // Data Receive process (H/W Rx socket buffer -> User's buffer)
        if(ret <= 0) return ret; // If the received data length <= 0, receive failed and process end
        size = (uint16_t) ret;

        // Data sentsize control
        sentsize = 0;
        while(size != sentsize) {
          ret = send(sn, buf+sentsize, size-sentsize); // Data send process (User's buffer -> Destination through H/W Tx socket buffer)
          if(ret < 0) { // Send Error occurred (sent data length < 0)
            close(sn); // socket close
            return ret;
          }
          sentsize += ret; // Don't care SOCKERR_BUSY, because it is zero.
        }
      }
      //////////////////////////////////////////////////////////////////////////////////////////////
      break;

    case SOCK_CLOSE_WAIT :
#ifdef _LOOPBACK_DEBUG_
      printf("%d:CloseWait\r\n",sn);
#endif
      if((ret=disconnect(sn)) != SOCK_OK) return ret;
#ifdef _LOOPBACK_DEBUG_
      printf("%d:Socket Closed\r\n", sn);
#endif
      break;

    case SOCK_INIT :
#ifdef _LOOPBACK_DEBUG_
      printf("%d:Try to connect to the %d.%d.%d.%d : %d\r\n", sn, destip[0], destip[1], destip[2], destip[3], destport);
#endif
      if( (ret = connect(sn, destip, destport)) != SOCK_OK) return ret;	//	Try to TCP connect to the TCP server (destination)
      break;

    case SOCK_CLOSED:
      close(sn);
      if((ret=socket(sn, Sn_MR_TCP, any_port++, 0x00)) != sn) {
        if(any_port == 0xffff) any_port = 50000;
        return ret; // TCP socket open with 'any_port' port number
      } 
#ifdef _LOOPBACK_DEBUG_
      printf("%d:TCP client loopback start\r\n",sn);
      printf("%d:Socket opened\r\n",sn);
#endif
      break;

    default:
      break;
  }
  return 1;
}

void network_init(void)
{
  wiz_NetTimeout gWIZNETTIME = {.retry_cnt = 3, .time_100us = 2000};
  ctlnetwork(CN_SET_TIMEOUT,(void*)&gWIZNETTIME);
  ctlnetwork(CN_GET_TIMEOUT, (void*)&gWIZNETTIME);
  printf("TIMEOUT: %d, %d\r\n", gWIZNETTIME.retry_cnt,gWIZNETTIME.time_100us);

	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);	
	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
	// Display Network Information
  uint8_t tmpstr[6];
	ctlwizchip(CW_GET_ID,(void*)tmpstr);
	printf("\r\n=== %s NET CONF ===\r\n",(char*)tmpstr);
	printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
    gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
	printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
	printf("======================\r\n");
}

void my_ip_assign(void)
{
   getIPfromDHCP(gWIZNETINFO.ip);
   getGWfromDHCP(gWIZNETINFO.gw);
   getSNfromDHCP(gWIZNETINFO.sn);
   getDNSfromDHCP(gWIZNETINFO.dns);
   gWIZNETINFO.dhcp = NETINFO_DHCP;
   /* Network initialization */
   network_init(); // apply from dhcp
   printf("DHCP LEASED TIME : %d Sec.\r\n", getDHCPLeasetime());
}

void my_ip_conflict(void)
{
	printf("CONFLICT IP from DHCP\r\n");
	//halt or reset or any...
	while(1); // this example is halt.
}

uint8_t buff[DATA_BUF_SIZE];
uint8_t sendbuf[1024]= {""};
int main(void)
{
  uint8_t memsize[2][8] = { {2,2,2,2,2,2,2,2},{2,2,2,2,2,2,2,2}};
  delay_init();

  uart_init(115200);
  led_init();

  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);		//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
  SPI1_Init(); // SPI1: PA5 PA6 PA7
  SPI1_SetSpeed(SPI_BaudRatePrescaler_2); //配置SPI1速度为最高
  register_wizchip();
  W5500_GPIO_Init(); // Initialize W5500,  RST INT SCS对应GPIO状态 并配置INT中断模式

  Load_Net_Parameters();

  /* WIZCHIP SOCKET Buffer initialize */
  if(ctlwizchip(0, (void*)memsize) == -1){
    printf("WIZCHIP Initialized fail.\r\n");
    while(1);
  }

  setSHAR(gWIZNETINFO.mac);
  DHCP_init(0, buff);
  reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);

  uint8_t dhcp_ret = DHCP_run();
  while(dhcp_ret != DHCP_IP_LEASED) {
    printf("Waiting DHCP: %d\r\n", dhcp_ret);
    led_toggle();
    delay_ms(500);
    dhcp_ret = DHCP_run();
  }

  uint8_t ret = 0;
  while(1) {
    if(usart_recv_flag) {
      usart_recv_flag = 0;
      ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
      printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\r\n",gWIZNETINFO.mac[0],gWIZNETINFO.mac[1],gWIZNETINFO.mac[2],\
      gWIZNETINFO.mac[3],gWIZNETINFO.mac[4],gWIZNETINFO.mac[5]);
      printf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
      printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0],gWIZNETINFO.gw[1],gWIZNETINFO.gw[2],gWIZNETINFO.gw[3]);
      printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0],gWIZNETINFO.sn[1],gWIZNETINFO.sn[2],gWIZNETINFO.sn[3]);
      printf("DNS: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0],gWIZNETINFO.dns[1],gWIZNETINFO.dns[2],gWIZNETINFO.dns[3]);
    }
    delay_ms(500);
    int rev = loopback_tcpc(1, buf, destip, destport);
    switch (rev) {
      case SOCK_OK:
        ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
        sprintf((char *)sendbuf,"Message from %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0],gWIZNETINFO.ip[1],gWIZNETINFO.ip[2],gWIZNETINFO.ip[3]);
        send(1, sendbuf, 30);
        send(1, buf, rev);
        break;
      case SOCKERR_TIMEOUT:
        printf("REV: TIMEOUT\r\n");
        break;
      case SOCKERR_SOCKSTATUS:
        printf("REV: SOCKSTATUS\r\n");
        break;
      default:
        printf("REV: SOCK ERROR\r\n");
    }

    /*if(rev) {
      sprintf((char *)sendbuf,"%d\r\n",rev);
      send(1, sendbuf, 10); 				
      send(1, buf, rev);
      memset(buf,'\0',rev);
    }*/
  }
}
