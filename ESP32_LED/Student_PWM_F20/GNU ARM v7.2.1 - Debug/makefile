################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/Source_Files/subdir.mk
-include src/subdir.mk
-include emlib/subdir.mk
-include CMSIS/EFM32PG12B/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: AB_I2C_Lab.axf

# Tool invocations
AB_I2C_Lab.axf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GNU ARM C Linker'
	arm-none-eabi-gcc -g3 -gdwarf-2 -mcpu=cortex-m4 -mthumb -T "AB_I2C_Lab.ld" -Xlinker --gc-sections -Xlinker -Map="AB_I2C_Lab.map" -mfpu=fpv4-sp-d16 -mfloat-abi=softfp --specs=nano.specs -u _printf_float -u _scanf_float -o AB_I2C_Lab.axf "./CMSIS/EFM32PG12B/startup_gcc_efm32pg12b.o" "./CMSIS/EFM32PG12B/system_efm32pg12b.o" "./emlib/em_acmp.o" "./emlib/em_adc.o" "./emlib/em_aes.o" "./emlib/em_assert.o" "./emlib/em_burtc.o" "./emlib/em_can.o" "./emlib/em_cmu.o" "./emlib/em_core.o" "./emlib/em_cryotimer.o" "./emlib/em_crypto.o" "./emlib/em_csen.o" "./emlib/em_dac.o" "./emlib/em_dbg.o" "./emlib/em_dma.o" "./emlib/em_ebi.o" "./emlib/em_emu.o" "./emlib/em_eusart.o" "./emlib/em_gpcrc.o" "./emlib/em_gpio.o" "./emlib/em_i2c.o" "./emlib/em_iadc.o" "./emlib/em_idac.o" "./emlib/em_int.o" "./emlib/em_lcd.o" "./emlib/em_ldma.o" "./emlib/em_lesense.o" "./emlib/em_letimer.o" "./emlib/em_leuart.o" "./emlib/em_mpu.o" "./emlib/em_msc.o" "./emlib/em_opamp.o" "./emlib/em_pcnt.o" "./emlib/em_pdm.o" "./emlib/em_prs.o" "./emlib/em_qspi.o" "./emlib/em_rmu.o" "./emlib/em_rtc.o" "./emlib/em_rtcc.o" "./emlib/em_se.o" "./emlib/em_system.o" "./emlib/em_timer.o" "./emlib/em_usart.o" "./emlib/em_vcmp.o" "./emlib/em_vdac.o" "./emlib/em_wdog.o" "./src/Source_Files/HW_delay.o" "./src/Source_Files/SI7021.o" "./src/Source_Files/app.o" "./src/Source_Files/ble.o" "./src/Source_Files/cmu.o" "./src/Source_Files/gpio.o" "./src/Source_Files/i2c.o" "./src/Source_Files/letimer.o" "./src/Source_Files/leuart.o" "./src/Source_Files/scheduler.o" "./src/Source_Files/sleep_routines.o" "./src/main.o" -Wl,--start-group -lgcc -lc -lnosys -Wl,--end-group
	@echo 'Finished building target: $@'
	@echo ' '

	@echo 'Building hex file: AB_I2C_Lab.hex'
	arm-none-eabi-objcopy -O ihex "AB_I2C_Lab.axf" "AB_I2C_Lab.hex"
	@echo ' '

	@echo 'Building bin file: AB_I2C_Lab.bin'
	arm-none-eabi-objcopy -O binary "AB_I2C_Lab.axf" "AB_I2C_Lab.bin"
	@echo ' '

	@echo 'Building s37 file: AB_I2C_Lab.s37'
	arm-none-eabi-objcopy -O srec "AB_I2C_Lab.axf" "AB_I2C_Lab.s37"
	@echo ' '

	@echo 'Running size tool'
	arm-none-eabi-size "AB_I2C_Lab.axf" -A
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) AB_I2C_Lab.axf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
