ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

TARGET      := dns-switcher
BUILD       := build
SOURCES     := source
INCLUDES    := source

LIBS        := -lfat -lwiikeyboard -lwiiuse -lbte -logc -lm
LIBDIRS     := $(CURDIR)/portlibs/ppc

CFLAGS      := -g -O2 -Wall -Wextra -ffunction-sections -D__wii__ \
               $(MACHDEP) $(INCLUDE)
CXXFLAGS    := $(CFLAGS)
LDFLAGS     := -g $(MACHDEP) -Wl,-Map,$(notdir $@).map,--gc-sections

# Optional extra controller support (needs ./scripts/build_controller_portlibs.sh
# run first to populate portlibs/ppc): Wii U Pro Controller/GamePad, DualShock 3.
WITH_WIIDRC    ?= 0
WITH_WUPC      ?= 0
WITH_SICKSAXIS ?= 0
ifeq ($(WITH_WIIDRC),1)
 CFLAGS += -DHAVE_WIIDRC
 LIBS := -lwiidrc $(LIBS)
endif
ifeq ($(WITH_WUPC),1)
 CFLAGS += -DHAVE_WUPC
 LDFLAGS += -Wl,-wrap,wiiuse_accept
 LIBS := -lwupc $(LIBS)
endif
ifeq ($(WITH_SICKSAXIS),1)
 CFLAGS += -DHAVE_SICKSAXIS
 LIBS := -lsicksaxis $(LIBS)
endif

ifneq ($(BUILD),$(notdir $(CURDIR)))
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH  := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))

export LD      := $(CC)
export OFILES  := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(SFILES:.s=.o)
export INCLUDE := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                  -I$(LIBOGC_INC) \
                  -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)

.PHONY: all clean run package

all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).dol

run: all
	wiiload $(TARGET).dol

package: all
	@mkdir -p dist/apps/dns-switcher
	@cp $(TARGET).dol meta/meta.xml meta/icon.png dist/apps/dns-switcher/

else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

-include $(DEPENDS)

endif
