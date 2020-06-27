#include "serial.h"

void initSerial(void)
{
	// 下面的寄存器 要在uart手册找
    SER_Disable();  // 就是关闭串口
    SER_Set_baud_rate( 38400 );	// 这个数值好像不能改，我改了后程序就不正常了。
    UART0->UARTLCR_H = UART_LCRH_WLEN_8;  // 8 bits, 1 stop bit, no parity and FIFO disabled,我修改过，5bit程序也正常工作
    	// 6:5 WLEN Word length. These bits indicate the number of data bits transmitted or received in a frame as follows:
    	//    b11 = 8 bits
    	//    b10 = 7 bits
    	//    b01 = 6 bits
    	//    b00 = 5 bits.

    SER_Enable();  // Enable UART and enable TX/RX
}

/*----------------------------------------------------------------------------
  Enable Serial Port
 *----------------------------------------------------------------------------*/
void SER_Enable(void)
{
	// 开启串口，串口使能，发送使能，接收使能
	// p62页有
    UART0->UARTCR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

/*----------------------------------------------------------------------------
  Disable Serial Port
 *----------------------------------------------------------------------------*/
void SER_Disable(void)
{
	// 关闭串口
	// p62页有
    UART0->UARTCR = 0x0;
}


/*----------------------------------------------------------------------------
  Set baud rate
 *----------------------------------------------------------------------------*/
// 这个不太懂
void SER_Set_baud_rate(uint32_t baud_rate) {
    uint32_t divider;
    uint32_t mod;
    uint32_t fraction;

    /*
     * Set baud rate
     *
     * IBRD = UART_CLK / (16 * BAUD_RATE)
     * FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
     * 手册33页有
     */
    divider   = UART0_CLK / (16 * baud_rate);
    mod       = UART0_CLK % (16 * baud_rate);
    fraction  = (((8 * mod) / baud_rate) >> 1) + (((8 * mod) / baud_rate) & 1);

    UART0->UARTIBRD = divider;
    UART0->UARTFBRD = fraction;
}

/*----------------------------------------------------------------------------
  Write character to Serial Port
 *----------------------------------------------------------------------------*/

// 把字符放在 UARTDR，之后UART就会将它传输到指定的位置，之后将 UART0->UARTFR 的0x20的位置置0，表示传输完成了。

void SER_PutChar(char c) {
	//  TXFF,Transmit FIFO full. P54
    while (UART0->UARTFR & 0x20);   // Wait for UART TX to become free，等待发送缓存为空(说明发送完毕了)，这是因为外设速度很慢，所以需要轮询

    UART0->UARTDR = c; // 把字符放到[0,7]位
    // 7:0 DATA Receive (read) data character.   P52
    // Transmit (write) data character
}

/*----------------------------------------------------------------------------
  Read character from Serial Port (blocking read)
 *----------------------------------------------------------------------------*/
char SER_GetChar (void) {
	// RXFE Receive FIFO empty.
    while (UART0->UARTFR & 0x10);  // Wait for a character to arrive， 等待接收缓存为空
    return UART0->UARTDR; // 字符放到[0,7]位，这句话就是返回这个字符
    // 7:0 DATA Receive (read) data character.
    // Transmit (write) data character
}

