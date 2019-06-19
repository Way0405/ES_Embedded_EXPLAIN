#include <stdint.h>
#include <stdio.h>
#include "reg.h"
#include "blink.h"
#include "usart.h"
#include "asm_func.h"

#define TASK_NUM 3  //總共有三個工作 
#define PSTACK_SIZE_WORDS 1024 //user stack size = 4 kB //每個工作分配到的記憶體大小

static uint32_t *psp_array[TASK_NUM];

void setup_systick(uint32_t ticks);

void init_task(unsigned int task_id, uint32_t *task_addr, uint32_t *psp_init)
{
	//*(psp_init-1)=(*(psp_init-1))|(0x01000000);	  //xPSR (bit 24, T bit, has to be 1 in Thumb state)
	*(psp_init-1)=UINT32_1<<24;	
	//sp_init-1為此process的psr (看記憶體配置圖)
	////psr第24設! 代表為thumb state

	*(psp_init-2)= (uint32_t) task_addr; //Return Address is being initialized to the task entry
	//return address擺此process code的位置
	psp_array[task_id] = psp_init-16;	//initialize psp_array (stack frame: 8 + r4 ~ r11: 8)
	//紀錄此process的結尾位置(同時也是下一個process的開頭位置)
}

void task0(void)//模擬process 0
{
	printf("[Task0] Start in unprivileged thread mode.\r\n\n");
	printf("[Task0] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink(LED_BLUE); //should not return
}

void task1(void)////模擬process 1
{
	printf("[Task1] Start in unprivileged thread mode.\r\n\n");
	printf("[Task1] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink(LED_GREEN); //should not return
}

void task2(void)//模擬process 2
{
	printf("[Task2] Start in unprivileged thread mode.\r\n\n");
	printf("[Task2] Control: 0x%x \r\n", (unsigned int)read_ctrl());

	blink(LED_ORANGE); //should not return
}

int main(void)
{
	init_usart1();//設定uart

	uint32_t user_stacks[TASK_NUM][PSTACK_SIZE_WORDS];

	//init user tasks //(task_id, *task_addr,  *psp_init)
	init_task(0, (uint32_t*)task0 , user_stacks[0]  +PSTACK_SIZE_WORDS);
	init_task(1, (uint32_t*)task1 , user_stacks[1]  +PSTACK_SIZE_WORDS);
	init_task(2, (uint32_t*)task2 , user_stacks[2]  +PSTACK_SIZE_WORDS);
	
	printf("[Kernel] Start in privileged thread mode.\r\n\n");

	printf("[Kernel] Setting systick...\r\n\n");
	setup_systick(168e6 / 8 / 100); //10 ms        
	//sys_clk為168M Hz
	//設為tth /8


	//start user task
	printf("[Kernel] Switch to unprivileged thread mode & start user task0 with psp.\r\n\n");
	start_user((uint32_t *)task0, user_stacks[0]);
	/*
	start_user:
	movs	lr,	r0   // r0= task0的地址
	msr	psp,	r1   // r1= user_stacks[0] 

	movs	r3,	#0b11
	msr	control,	r3  // control 更改成user mode
	isb

	bx	lr	//跳到 task0
	 */

	while (1) //should not go here
		;
}

void setup_systick(uint32_t ticks)
{
	// set reload value
	//設定 倒計時的次數
	WRITE_BITS(SYST_BASE + SYST_RVR_OFFSET, SYST_RELOAD_23_BIT, SYST_RELOAD_0_BIT, ticks - 1);

	// uses external reference clock
	//用外部CLK當來源
	CLEAR_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_CLKSOURCE_BIT);

	// enable systick exception
	//enable 倒計時後觸發 interrupt
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_TICKINT_BIT);

	// enable systick
	//開始倒計時
	SET_BIT(SYST_BASE + SYST_CSR_OFFSET, SYST_ENABLE_BIT);
}

uint32_t *sw_task(uint32_t *psp)
{
	static unsigned int curr_task_id = 0;

	psp_array[curr_task_id] = psp; //save current psp

	if (++curr_task_id > TASK_NUM - 1) //get next task id
		curr_task_id = 0;

	return psp_array[curr_task_id]; //return next psp
}