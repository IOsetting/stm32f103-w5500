# stm32f103-w5500

A working demo of W5500 Ethernet module running on STM32F103C8T6.

# Components Required

* STM32F103C8 board
* W5500 Ethernet RJ45 module
* USB2TTL: PL2303 or CH340 or FT232 or CP2102
* ST-Link or J-LInk
* Router/Switch Hub for Ethernet connection

# Circuit Diagram and connections

Refer below table to connect ESP-01S pins with STM32 pins:

| W5500     | STM32    | USB2TTL      |
| --------- | -------- | ------------ |
| GND       | GND      | GND          |
| VCC       | 3.3V     |              |
| RST       |          |              |
| INT       |          |              |
| SCS       | PA0      |              |
| CLK       | PA5      |              |
| MISO      | PA6      |              |
| MOSI      | PA7      |              |
|           | PA9      | RX           |
|           | PA10     | TX           |


# Working & Code Explanation

Use ST-Link to upload the code to STM32.  

Connect ST-Link with STM32F103C8T6 Development Board
```
G   -- GND
CLK -- SWCLK
IO  -- SWDIO
V3  -- 3.3V
```

## Configuration

### MAC, IP, Mask, Gateway and Remote Server IP

Edit in main.c
```c
void Load_Net_Parameters(void)
{
	gWIZNETINFO.gw[0] = 192; //Gateway
	gWIZNETINFO.gw[1] = 168;
	gWIZNETINFO.gw[2] = 1;
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
	gWIZNETINFO.ip[2]=1;
	gWIZNETINFO.ip[3]=204;

	gWIZNETINFO.dhcp = NETINFO_STATIC;
}
uint8_t destip[4] = {192, 168, 1, 210};
uint16_t destport = 3333;
```

### Log Output

Add `_DHCP_DEBUG_` and `_LOOPBACK_DEBUG_` to preprocessor symbols, this will enable the debug log output (in UART1).

## Compile 

Include Paths
```
..\libraries\CMSIS\CM3\CoreSupport;..\libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x;..\libraries\CMSIS\CM3\DeviceSupport\ST\STM32F10x\startup\arm;..\libraries\STM32F10x_StdPeriph_Driver\inc;..\hardware;..\user
```
Compile Control String
```bash
--c99 --gnu -c --cpu Cortex-M3 -D__MICROLIB -g -O0 --apcs=interwork --split_sections -I ../libraries/CMSIS/CM3/CoreSupport -I ../libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x -I ../libraries/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/arm -I ../libraries/STM32F10x_StdPeriph_Driver/inc -I ../hardware -I ../user
-I./RTE/_stm32f103c8w5500
-IC:/Keil_v5/ARM/PACK/Keil/STM32F1xx_DFP/2.3.0/Device/Include
-IC:/Keil_v5/ARM/CMSIS/Include
-D__UVISION_VERSION="525" -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -D_DHCP_DEBUG_ -D_LOOPBACK_DEBUG_
-o .\Objects\*.o --omf_browse .\Objects\*.crf --depend .\Objects\*.d
```

## Test

* Ping test: Ping the IP address you defined in main.c
* TCP client test: Run `nc -l 3333` to listen port 3333 on server IP, and restart stm32 board

## Reference

* https://github.com/Wiznet/ioLibrary_Driver