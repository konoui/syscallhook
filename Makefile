obj-m := syscallhook.o
syscallhook-objs := fake.o main_linux.o
ccflags-y := -O0 -Wno-format -Wno-declaration-after-statement -Wno-unused-function -std=gnu99

BIN := syscallhook.ko
KVERSION := $(shell uname -r)
KDIR := /lib/modules/$(KVERSION)
KBUILD := $(KDIR)/build
PWD := $(shell pwd)
MAKEFLAGS += --no-print-directory

all:
	@make -C $(KBUILD) M=$(PWD) modules

clean:
	@make -C $(KBUILD) M=$(PWD) clean

load:
	@echo Loading $(BIN)
	@insmod $(BIN)

unload:
	@echo Unloading $(BIN)
	@rmmod $(BIN)

