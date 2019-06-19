.syntax unified
/*
回傳數值時,是把數字放到r0
*/

.global	read_ctrl
read_ctrl:
	mrs	r0,	control
	bx	lr

.global	start_user
start_user:
	movs	lr,	r0   // r0= task0的地址
	msr	psp,	r1   // r1= user_stacks[0] 

	movs	r3,	#0b11
	msr	control,	r3  // control 更改成user mode
	isb

	bx	lr

.type systick_handler, %function
.global systick_handler
systick_handler:
	//save lr (EXC_RETURN) to main stack
	push {lr}

	//save r4-r11 to user stack
	mrs	r0,	psp
	stmdb	r0!,	{r4-r11}

	//pass psp of curr task by r0 and get psp of the next task
	bl	sw_task
	//psp of the next task is now in r0

	//restore r4~r11 from stack of the next task
	ldmia	r0!,	{r4-r11}

	//modify psp
	msr psp, r0

	//restore lr (EXC_RETURN)
	pop {lr}

	bx	lr

/*
	.global	read_sp
read_sp:
	mov	r0,sp  //回傳sp
	bx lr

.global	read_msp
read_msp:
	mrs r0,msp //回傳msp
	bx lr
	
.global	read_psp
read_psp:
	mrs r0,psp //回傳psp
	bx lr

.global	read_ctrl
read_ctrl:
	mrs r0,control //回傳control
	bx lr
*/
