#include "serial.h"

void initSerial(void)
{
	// ����ļĴ��� Ҫ��uart�ֲ���
    SER_Disable();  // ���ǹرմ���
    SER_Set_baud_rate( 38400 );	// �����ֵ�����ܸģ��Ҹ��˺����Ͳ������ˡ�
    UART0->UARTLCR_H = UART_LCRH_WLEN_8;  // 8 bits, 1 stop bit, no parity and FIFO disabled,���޸Ĺ���5bit����Ҳ��������
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
	// �������ڣ�����ʹ�ܣ�����ʹ�ܣ�����ʹ��
	// p62ҳ��
    UART0->UARTCR = UART_CR_UARTEN | UART_CR_TXE | UART_CR_RXE;
}

/*----------------------------------------------------------------------------
  Disable Serial Port
 *----------------------------------------------------------------------------*/
void SER_Disable(void)
{
	// �رմ���
	// p62ҳ��
    UART0->UARTCR = 0x0;
}


/*----------------------------------------------------------------------------
  Set baud rate
 *----------------------------------------------------------------------------*/
// �����̫��
void SER_Set_baud_rate(uint32_t baud_rate) {
    uint32_t divider;
    uint32_t mod;
    uint32_t fraction;

    /*
     * Set baud rate
     *
     * IBRD = UART_CLK / (16 * BAUD_RATE)
     * FBRD = ROUND((64 * MOD(UART_CLK,(16 * BAUD_RATE))) / (16 * BAUD_RATE))
     * �ֲ�33ҳ��
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

// ���ַ����� UARTDR��֮��UART�ͻὫ�����䵽ָ����λ�ã�֮�� UART0->UARTFR ��0x20��λ����0����ʾ��������ˡ�

void SER_PutChar(char c) {
	//  TXFF,Transmit FIFO full. P54
    while (UART0->UARTFR & 0x20);   // Wait for UART TX to become free���ȴ����ͻ���Ϊ��(˵�����������)��������Ϊ�����ٶȺ�����������Ҫ��ѯ

    UART0->UARTDR = c; // ���ַ��ŵ�[0,7]λ
    // 7:0 DATA Receive (read) data character.   P52
    // Transmit (write) data character
}

/*----------------------------------------------------------------------------
  Read character from Serial Port (blocking read)
 *----------------------------------------------------------------------------*/
char SER_GetChar (void) {
	// RXFE Receive FIFO empty.
    while (UART0->UARTFR & 0x10);  // Wait for a character to arrive�� �ȴ����ջ���Ϊ��
    return UART0->UARTDR; // �ַ��ŵ�[0,7]λ����仰���Ƿ�������ַ�
    // 7:0 DATA Receive (read) data character.
    // Transmit (write) data character
}

