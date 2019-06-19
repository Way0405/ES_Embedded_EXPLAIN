CROSS-COMPILER = arm-none-eabi-

all: contexswitch.bin

contexswitch.bin: main.c newlib_port.c usart.c blink.c startup.c vector_table.s asm_func.s
	$(CROSS-COMPILER)gcc -std=c11 -Wall -mcpu=cortex-m4 -mthumb -nostartfiles -T stm32f4.ld main.c newlib_port.c usart.c blink.c startup.c vector_table.s asm_func.s -o contexswitch.elf
	$(CROSS-COMPILER)objcopy -O binary contexswitch.elf contexswitch.bin

flash:
	st-flash --reset write contexswitch.bin 0x8000000

clean:
	rm -f *.o *.elf *.bin
