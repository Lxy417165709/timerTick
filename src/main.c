#include <stdio.h>
#include "gicv3_basic.h"
#include "generic_timer.h"
#include "system_counter.h"
#include "serial.h"
#include "spider.h"

extern uint32_t getAffinity(void);
extern void changeColorOfLED();

void initGIC(void);
void initCPUInterface(void);
void initInteruptOfRedistributor(uint32_t);
void initRedistributor(void);
void initPhysicalTimer(void);
void initGIC(void);
void spin(void);


#define HzOfTick (getCNTFID(SYSTEM_COUNTER_CNTCR_FREQ0))



int main(void) {
	initSerial();
	initGIC();				// ��Ҫ�� distributor�ڲ���������
	initRedistributor();	// ��redistributor�ڲ���������.. ���嵽���жϡ��жϵķ���...
	initCPUInterface();// �� CPU interface�ڲ���������... ���嵽���顢�ж����ȼ�
	initSystemCounter(
			SYSTEM_COUNTER_CNTCR_nHDBG,
			SYSTEM_COUNTER_CNTCR_FREQ0,
			SYSTEM_COUNTER_CNTCR_nSCALE
	);

	initPhysicalTimer();	// understand
	print("all init.\n\r\n");


	// NOTE:
	// This code assumes that the IRQ and FIQ exceptions
	// have been routed to the appropriate Exception level
	// and that the PSTATE masks are clear.  In this example
	// this is done in the startup.s file

	spin();
	return 1;
}



void initGIC(void){
	setGICAddr((void*)0x2F000000, (void*)0x2F100000);	// ��Щ��ַ�ǹ̶��ģ�������Help Content�ҵ�
	enableGIC();										// ʹ��GIC
}

void initRedistributor(void) {
	// Get the ID of the Redistributor connected to this PE
	uint32_t rd = getRedistID(getAffinity());	// ����б��ʵ� 0.0..

	// Mark this core as begin active
	wakeUpRedist(rd);
	initInteruptOfRedistributor(rd);
}


// ����������CPUInterface������
void initCPUInterface(void) {
	// �����̫��
	// Configure the CPU interface
	// This assumes that the SRE bits are already set (��������ʱ���������)
	setPriorityMask(0xFF);	// �������ȼ������ȼ��͵Ľ��޷����͡� (���ȼ�������ֵ�������С), ���Ѿ���֤����

	enableGroup0Ints();	// ���ע���˾Ͳ���������������������0�жϣ����û���ã��Ͳ�ת���������жϰ� (�ĵ�д����1�� ��0�ж�ʹ�ܣ���0����0�жϱ����á���)
	// ���Ӧ�û����ж���������й�


	//enableGroup1Ints(); // ���ע���˻�����������
}


// ��������漰���� redistrubutor �� distrubutor�ļĴ���
void initInteruptOfRedistributor(uint32_t rd) {
	// Secure Physical Timer      (INTID 29)
	// Secure Physical Timer �������жϺ�Ϊ 29�������������� 29��
	// ������: �ж�ʹ�ܡ������ж����ȼ��������ж��顢CPUѡ��(�������û����...,����ָ����rd������)

	enableInt(29, rd);
	setIntPriority(29, rd, 0);	// ��Ҳ��CPU interface�������й�
	setIntGroup(29, rd, 0);	// ���������̫��.. ���������ж��飬֮��cpu interface���������飬���������ͨ����ͨ�������������

}




void initPhysicalTimer(void) {
	// Configure the Secure Physical Timer
	// This uses the CVAL/comparator to set an absolute time for the timer to fire
	// �����漰��3���Ĵ�����
	// 	CNTPS_CTL_EL1       һ�����ƼĴ������ܿ����Ƿ����Ƚ�
	// 	CNTPS_CVAL_EL1    һ���Ƚ�ֵ�����CNTPCT_EL0��click >= ��ֵ�������һ�� 29���ж�
	//  CNTPCT_EL0		����click

	uint32_t current_time = getPhysicalCount();
	setSEL1PhysicalCompValue(current_time + 10000);	// ��䲻̫������ɾ���˻������У�Ӧ���Ƿ�ֹpysicalTimer һ�����ͷ����жϰɣ�
	// all init.:00   ��ʱ������е㲻����
	// all init. �����������
	setSEL1PhysicalTimerCtrl(CNTPS_CTL_ENABLE);
}





void spin() {
	while(1);
}





void displayClock(void) {
	char msg[128];
	int tick = getPhysicalCount()/HzOfTick;	// ��ʱ���ġ�ʱ�䡱 / Ƶ�� �� ����������
	sprintf(msg, "now Time: %02d:%02d\r", tick / 60, tick % 60);
	print(msg);
}



void resetTimer(void){
	uint64_t current_time;
	setSEL1PhysicalTimerCtrl(0);						// �ر�
	current_time = getPhysicalCount();
	setSEL1PhysicalCompValue(current_time + HzOfTick);	// ����������һ��ͷ��ж�
	setSEL1PhysicalTimerCtrl(CNTPS_CTL_ENABLE);		    // ����
}


void timerTick(void) {
	resetTimer();
	changeColorOfLED();
	displayClock();
}



void fiqHandler(void) {
	unsigned int ID;

	// Read the IAR to get the INTID of the interrupt taken
	ID = readIARGrp0();

	printf("Get FIQ: %d\n", ID);

	switch (ID) {
		case 29:
			timerTick();
			break;
		default:
			printf("FIQ: PANIC, ERROR\n");
			return;
	}

	// Write EOIR to deactivate interrupt
	writeEOIGrp0(ID);
}




