BOARD ?= b-l072z-lrwan1

APPLICATION = test
RIOTBASE ?= $(CURDIR)/RIOT

LORA_DRIVER ?= sx1276
LORA_REGION ?= EU868

USEPKG += semtech-loramac
USEMODULE += $(LORA_DRIVER)

# load loramac RX if board supports it
ifeq (,$(filter $(BOARD),$(BOARD_WITHOUT_LORAMAC_RX)))
  USEMODULE += semtech_loramac_rx
endif

USEMODULE += auto_init_loramac
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += fmt
USEMODULE += xtimer
USEMODULE += random

FEATURES_OPTIONAL += periph_eeprom

# Default IotLab Config to run the test
ifneq (,$(filter iotlab%,$(MAKECMDGOALS)))
  IOTLAB_NODES ?= 1
  IOTLAB_TYPE  ?= st-lrwan1:sx1276
  IOTLAB_SITE  ?= saclay
  include $(RIOTBASE)/dist/testbed-support/Makefile.iotlab
endif

include $(RIOTBASE)/Makefile.include
