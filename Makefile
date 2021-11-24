CC=sdcc
CFLAGS=-mstm8 
INCLUDEPATH=../../../libraries/stm8s-sdcc/inc/
SRCPATH=../../../libraries/stm8s-sdcc/src/
DEFINES= STM8S103
STM8_ROUTINES=/./../../libraries/stm8_routines/
SOURCE=main
OUTPUT_DIR=build/
all:
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SRCPATH)stm8s_clk.c
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SRCPATH)stm8s_uart1.c
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SRCPATH)stm8s_gpio.c
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SRCPATH)stm8s_tim2.c
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SRCPATH)stm8s_tim4.c
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SRCPATH)stm8s_adc1.c
	$(CC) -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) debug_lib.c
	$(CC) $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SOURCE).c $(OUTPUT_DIR)stm8s_clk.rel $(OUTPUT_DIR)stm8s_uart1.rel $(OUTPUT_DIR)stm8s_gpio.rel $(OUTPUT_DIR)stm8s_tim2.rel $(OUTPUT_DIR)stm8s_tim4.rel $(OUTPUT_DIR)stm8s_adc1.rel $(OUTPUT_DIR)debug_lib.rel
test1:
	$(CC) -E -c $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) onewire.c
test2:
	$(CC) -E $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SOURCE).c $(OUTPUT_DIR)stm8s_clk.rel $(OUTPUT_DIR)stm8s_uart1.rel $(OUTPUT_DIR)stm8s_gpio.rel $(OUTPUT_DIR)onewire.rel $(OUTPUT_DIR)debug_lib.rel
#all: flash
#compile:
#	mkdir -p $(OUTPUT_DIR)
#	$(CC) $(CFLAGS) -I $(INCLUDEPATH) -D $(DEFINES) -o $(OUTPUT_DIR) $(SOURCE).c 
#prepare:
#	packihx $(OUTPUT_DIR)/$(SOURCE).ihx > $(OUTPUT_DIR)/$(SOURCE).hex
#clean:
#	rm -Rrf $(OUTPUT_DIR)
#erase:
#	vsprog -cstm8s103f3 -ms -oe -V"tvcc.set 3300"
#	vsprog -V"tvcc.set 0" -V"tvcc.set 3300"
#flash: compile prepare erase
#	vsprog -cstm8s103f3 -ms -owf -I $(OUTPUT_DIR)/$(SOURCE).hex -V"tvcc.set 3300"
#	vsprog -V"tvcc.set 0" -V"tvcc.set 3300"
