########################################################################
# Author: Stevin Liang <liangzl@yutong.com>
#         This Makefile is for lpc40xx with freertos 9.0.
########################################################################
TARGET := lpc4078-freertos
TARGET_ELF := $(TARGET).elf
TARGET_HEX := $(TARGET).hex
TARGET_MAP := $(TARGET).map
TARGET_SIZE := $(TARGET).siz
TARGET_OBJDUMP := $(TARGET).S

# Selecting Cpu Core
CPU_CORE := cortex-m4
ifeq ($(CPU_CORE), cortex-m4)
	FPU_FLAGS := -mfloat-abi=hard -mfpu=fpv5-sp-d16
endif
ARCH_FLAGS=-mcpu=$(CPU_CORE) -mthumb $(FPU_FLAGS)

# Use newlib-nano. To disable it, specify USE_NANO=
USE_NANO=--specs=nano.specs

# Use semihosting or not
USE_SEMIHOST=--specs=rdimon.specs
USE_NOHOST=--specs=nosys.specs

# Specify Cross Compiler
CROSS_COMPILE := arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
AR = $(CROSS_COMPILE)ar
LD = $(CROSS_COMPILE)ld
SIZE = $(CROSS_COMPILE)size
STRIP = $(CROSS_COMPILE)strip
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump

# Include directories
INCLUDES := -I ./system/base/inc \
	    -I ./system/board/inc \
	    -I ./system/chip/inc \
	    -I ./system/drivers/inc \
	    -I ./system/freertos/inc \
	    -I ./system/startup/inc \
	    -I ./usr/inc

# Link Scripts for Linker
LDSCRIPTS_DIR := -L. -L./ldscripts
LDSCRIPTS += $(LDSCRIPTS_DIR) -T mem.ld -T sections.ld

#Libraries for Linking
LIBS += -L ./system/chip/libs -lusbd_40xx_lib
# MACRO Sysmbols passing to Compiler
SYSMBOLS := -DCORE_M4 -D__USE_LPCOPEN

HOST_OS := $(shell uname -a | awk '{print $$15" "$$14" "$$1}')

DIRS := $(shell find . -type d)
C_FILES := $(foreach dir, $(DIRS), $(wildcard $(dir)/*.c))
OBJS := $(patsubst %.c, %.o, $(C_FILES))
OBJDS := $(patsubst %.c, %.d, $(C_FILES))

CFLAGS := $(ARCH_FLAGS) -Og -fmessage-length=0 -fno-builtin\
	  -fsigned-char -ffunction-sections -fdata-sections -ffreestanding \
	  -Wuninitialized -Wall -Wno-unused-parameter -Wno-sign-compare \
	  -Wno-unused-but-set-variable -Wextra -Wno-switch -Wno-old-style-declaration \
	  -Wpointer-arith -Wno-shadow -Wno-incompatible-pointer-types -Wlogical-op \
	  -Waggregate-return -Wfloat-equal -g3 $(INCLUDES) $(SYSMBOLS) -std=gnu11 \
	  -Wbad-function-cast -MMD -MP

LFLAGS := $(ARCH_FLAGS) -Og $(USE_NANO) $(USE_NOHOST) $(LIBS) -ffunction-sections \
	  -fdata-sections -g3 -ffreestanding -nostartfiles -Xlinker $(LDSCRIPTS) \
	  -Wl,--gc-sections -Wl,-Map,"$(TARGET_MAP)"

RM := rm -rf

SECONDARY_FLASH += $(TARGET_HEX)
SECONDARY_SIZE += $(TARGET_SIZE)
SECONDARY_OBJDUMP += $(TARGET_OBJDUMP)

MAKEDEPS=Makefile

all: first-outputs $(TARGET_ELF) secondary-outputs

first-outputs:
	@echo '---------------------------------------------------'
	@echo '  Project Name:  $(TARGET)'
	@echo '  Cross Compile: $(CC)'
	@echo '  Target CORE:   $(CPU_CORE)'
	@echo '  Host OS:       $(HOST_OS)'
	@echo '---------------------------------------------------'
	@echo ''
# MAKEDEPS is for when update Makefile, compile all files
%.o : %.c $(MAKEDEPS)
	@echo '  CC      $@'
	@$(CC) $(CFLAGS) -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o $@ $<

$(TARGET_ELF): $(OBJS)
	@echo ' '
	@echo '  Linking target: $@'
	@echo '  Invoking: Cross ARM C Linker'
	@echo '  LD      $@'
	@$(CC) $(LFLAGS) -o $@ $(OBJS)
	@echo '  Finished Link target: $@'
	@echo ' '

$(TARGET_HEX): $(TARGET_ELF)
	@echo '  Generating Flash Image'
	@echo '  Invoking: Cross ARM GNU Create Flash Image'
	@echo '  OBJCOPY      $@'
	@$(OBJCOPY) -O ihex $(TARGET_ELF) $(TARGET_HEX)
	@echo '  Finished Generate Flash Image: $@'
	@echo ' '

$(TARGET_SIZE): $(TARGET_ELF)
	@echo '  Invoking: Cross ARM GNU Print Size'
	@echo '  SIZE      $@'
	@$(SIZE) --format=berkeley $<
	@echo '  Finished building: $@'
	@echo ' '

$(TARGET_OBJDUMP): $(TARGET_ELF)
	@echo '  Invoking: Cross ARM GNU Disassemble'
	@echo '  OBJDUMP      $<'
	@$(OBJDUMP) -d  $< > $@
	@echo '  Finished Disassemble: $@'
	@echo ' '
# Other Targets

clean:
	@-$(RM) $(OBJS) $(OBJDS) $(TARGET_ELF) $(TARGET_HEX) $(TARGET_MAP) $(TARGET_OBJDUMP)
	@echo '  CLEAN  $(TARGET) Done'

secondary-outputs: $(SECONDARY_FLASH) $(SECONDARY_SIZE) $(SECONDARY_OBJDUMP)

.PHONY: all clean
