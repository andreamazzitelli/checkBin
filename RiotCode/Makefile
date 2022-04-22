APPLICATION = checkBin

BOARD ?= b-l072z-lrwan1

RIOTBASE ?= $(CURDIR)/RIOT

LORA_DRIVER ?= sx1276
LORA_REGION ?= EU868
USEPKG += semtech-loramac
USEMODULE += $(LORA_DRIVER)
USEMODULE += semtech_loramac_rx
USEMODULE += auto_init_loramac

#check if needed
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += fmt

USEMODULE += periph_gpio
USEMODULE += xtimer
USEMODULE += periph_gpio_irq
USEMODULE += servo
USEMODULE += hd44780

FEATURES_OPTIONAL += periph_eeprom
FEATURES_REQUIRED = periph_gpio

include $(RIOTBASE)/Makefile.include