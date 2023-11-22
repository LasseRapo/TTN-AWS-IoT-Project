BOARD ?= b-l072z-lrwan1

APPLICATION ?= lorawan_sensors

RIOTBASE ?= $(CURDIR)/../RIOT

USEMODULE += ztimer_msec
USEMODULE += hts221
USEMODULE += sx1276
USEMODULE += lps22hb

USEPKG += semtech-loramac

LORA_REGION ?= EU868 # Default region Europe and default band 868MHz

include $(RIOTBASE)/Makefile.include
