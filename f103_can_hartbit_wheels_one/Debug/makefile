################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include Drivers/STM32F1xx_HAL_Driver/Src/subdir.mk
-include Core/Startup/subdir.mk
-include Core/Src/subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

OPTIONAL_TOOL_DEPS := \
$(wildcard ../makefile.defs) \
$(wildcard ../makefile.init) \
$(wildcard ../makefile.targets) \


BUILD_ARTIFACT_NAME := f103_can_hartbit_wheels_one
BUILD_ARTIFACT_EXTENSION := elf
BUILD_ARTIFACT_PREFIX :=
BUILD_ARTIFACT := $(BUILD_ARTIFACT_PREFIX)$(BUILD_ARTIFACT_NAME)$(if $(BUILD_ARTIFACT_EXTENSION),.$(BUILD_ARTIFACT_EXTENSION),)

# Add inputs and outputs from these tool invocations to the build variables 
EXECUTABLES += \
f103_can_hartbit_wheels_one.elf \

MAP_FILES += \
f103_can_hartbit_wheels_one.map \

SIZE_OUTPUT += \
default.size.stdout \

OBJDUMP_LIST += \
f103_can_hartbit_wheels_one.list \

OBJCOPY_HEX += \
f103_can_hartbit_wheels_one.hex \

OBJCOPY_BIN += \
f103_can_hartbit_wheels_one.bin \


# All Target
all: main-build

# Main-build Target
main-build: f103_can_hartbit_wheels_one.elf secondary-outputs

# Tool invocations
f103_can_hartbit_wheels_one.elf f103_can_hartbit_wheels_one.map: $(OBJS) $(USER_OBJS) C:\Users\user\Desktop\STM32_progect_all\f103_can_hartbit_wheels_one\STM32F103C8TX_FLASH.ld makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-gcc -o "f103_can_hartbit_wheels_one.elf" @"objects.list" $(USER_OBJS) $(LIBS) -mcpu=cortex-m3 -T"C:\Users\user\Desktop\STM32_progect_all\f103_can_hartbit_wheels_one\STM32F103C8TX_FLASH.ld" --specs=nosys.specs -Wl,-Map="f103_can_hartbit_wheels_one.map" -Wl,--gc-sections -static --specs=nano.specs -mfloat-abi=soft -mthumb -Wl,--start-group -lc -lm -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

default.size.stdout: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-size  $(EXECUTABLES)
	@echo 'Finished building: $@'
	@echo ' '

f103_can_hartbit_wheels_one.list: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objdump -h -S $(EXECUTABLES) > "f103_can_hartbit_wheels_one.list"
	@echo 'Finished building: $@'
	@echo ' '

f103_can_hartbit_wheels_one.hex: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O ihex $(EXECUTABLES) "f103_can_hartbit_wheels_one.hex"
	@echo 'Finished building: $@'
	@echo ' '

f103_can_hartbit_wheels_one.bin: $(EXECUTABLES) makefile objects.list $(OPTIONAL_TOOL_DEPS)
	arm-none-eabi-objcopy  -O binary $(EXECUTABLES) "f103_can_hartbit_wheels_one.bin"
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) default.size.stdout f103_can_hartbit_wheels_one.bin f103_can_hartbit_wheels_one.elf f103_can_hartbit_wheels_one.hex f103_can_hartbit_wheels_one.list f103_can_hartbit_wheels_one.map
	-@echo ' '

secondary-outputs: $(SIZE_OUTPUT) $(OBJDUMP_LIST) $(OBJCOPY_HEX) $(OBJCOPY_BIN)

fail-specified-linker-script-missing:
	@echo 'Error: Cannot find the specified linker script. Check the linker settings in the build configuration.'
	@exit 2

warn-no-linker-script-specified:
	@echo 'Warning: No linker script specified. Check the linker settings in the build configuration.'

.PHONY: all clean dependents main-build fail-specified-linker-script-missing warn-no-linker-script-specified

-include ../makefile.targets
