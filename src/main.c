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
	initGIC();				// 主要对 distributor内部进行设置
	initRedistributor();	// 对redistributor内部进行设置.. 具体到了中断、中断的分组...
	initCPUInterface();// 对 CPU interface内部进行设置... 具体到了组、中断优先级
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
	setGICAddr((void*)0x2F000000, (void*)0x2F100000);	// 这些地址是固定的，可以再Help Content找到
	enableGIC();										// 使能GIC
}

void initRedistributor(void) {
	// Get the ID of the Redistributor connected to this PE
	uint32_t rd = getRedistID(getAffinity());	// 这个有被问到 0.0..

	// Mark this core as begin active
	wakeUpRedist(rd);
	initInteruptOfRedistributor(rd);
}


// 里面是设置CPUInterface的属性
void initCPUInterface(void) {
	// 这个不太懂
	// Configure the CPU interface
	// This assumes that the SRE bits are already set (所以这有时序耦合性了)
	setPriorityMask(0xFF);	// 设置优先级，优先级低的将无法发送。 (优先级随着数值增大而减小), 我已经验证过了

	enableGroup0Ints();	// 这个注释了就不正常咯，功能是启用组0中断，如果没启用，就不转发这个组的中断吧 (文档写：设1是 组0中断使能，设0是组0中断被禁用。。)
	// 这个应该还和中断组的设置有关


	//enableGroup1Ints(); // 这个注释了还能正常工作
}


// 这个函数涉及到了 redistrubutor 和 distrubutor的寄存器
void initInteruptOfRedistributor(uint32_t rd) {
	// Secure Physical Timer      (INTID 29)
	// Secure Physical Timer 发出的中断号为 29，所以这里配置 29。
	// 步骤是: 中断使能、设置中断优先级、设置中断组、CPU选择(这个好像没看到...,估计指定了rd就是了)

	enableInt(29, rd);
	setIntPriority(29, rd, 0);	// 这也和CPU interface的设置有关
	setIntGroup(29, rd, 0);	// 这个函数不太懂.. 这里设置中断组，之后cpu interface还会进行审查，如果该组能通过就通过，否则就凉凉

}




void initPhysicalTimer(void) {
	// Configure the Secure Physical Timer
	// This uses the CVAL/comparator to set an absolute time for the timer to fire
	// 这里涉及到3个寄存器，
	// 	CNTPS_CTL_EL1       一个控制寄存器，能控制是否开启比较
	// 	CNTPS_CVAL_EL1    一个比较值，如果CNTPCT_EL0的click >= 其值，则产生一个 29号中断
	//  CNTPCT_EL0		保留click

	uint32_t current_time = getPhysicalCount();
	setSEL1PhysicalCompValue(current_time + 10000);	// 这句不太懂，我删除了还能运行，应该是防止pysicalTimer 一开启就发出中断吧？
	// all init.:00   此时输出会有点不正常
	// all init. 这个是正常的
	setSEL1PhysicalTimerCtrl(CNTPS_CTL_ENABLE);
}





void spin() {
	while(1);
}





void displayClock(void) {
	char msg[128];
	int tick = getPhysicalCount()/HzOfTick;	// 计时器的“时间” / 频率 就 等于秒数了
	sprintf(msg, "now Time: %02d:%02d\r", tick / 60, tick % 60);
	print(msg);
}



void resetTimer(void){
	uint64_t current_time;
	setSEL1PhysicalTimerCtrl(0);						// 关闭
	current_time = getPhysicalCount();
	setSEL1PhysicalCompValue(current_time + HzOfTick);	// 这里设置下一秒就发中断
	setSEL1PhysicalTimerCtrl(CNTPS_CTL_ENABLE);		    // 开启
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




