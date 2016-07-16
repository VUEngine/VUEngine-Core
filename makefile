# Makefile taken from Wikipedia.org

# Specify the main target
TARGET = libvbjae

# Default build type
#TYPE = debug
TYPE = release
#TYPE = preprocessor

# compiler
COMPILER = 4.7
COMPILER_OUTPUT = c
COMPILER_NAME = v810

ifeq ($(COMPILER), 4.7)
COMPILER_NAME = v810-nec-elf32
endif

GCC = $(COMPILER_NAME)-gcc
AS = $(COMPILER_NAME)-as
AR = $(COMPILER_NAME)-ar
OBJCOPY = $(COMPILER_NAME)-objcopy
OBJDUMP = $(COMPILER_NAME)-objdump


# my home
VBJAENGINE = $(VBDE)libs/vbjaengine

# Which directories contain source files
DIRS := $(shell find $(VBJAENGINE)/assets $(VBJAENGINE)/source $(VBJAENGINE)/lib/compiler/extra -type d -print)

# Which libraries are linked
LIBS =
ROMHEADER = lib/vb.hdr
# Dynamic libraries
DLIBS =

# Obligatory headers
CONFIG_FILE = $(VBJAENGINE)/config.h

ESSENTIALS =  -include $(VBJAENGINE)/libvbjae.h


# The next blocks changes some variables depending on the build type
ifeq ($(TYPE), debug)
LDPARAM = -fno-builtin -ffreestanding
CCPARAM = -nodefaultlibs -mv810 -Wall -O0 -Winline -std=gnu99 -fstrict-aliasing -include $(CONFIG_FILE) $(ESSENTIALS)
MACROS = __DEBUG __TOOLS
endif

ifeq ($(TYPE), release)
LDPARAM =
CCPARAM = -nodefaultlibs -mv810 -finline-functions -Wall -O2 -Winline -std=gnu99 -fstrict-aliasing -include $(CONFIG_FILE) $(ESSENTIALS)
MACROS =
endif

ifeq ($(TYPE), release-tools)
LDPARAM =
CCPARAM = -nodefaultlibs -mv810 -finline-functions -Wall -O2 -Winline -std=gnu99 -fstrict-aliasing -include $(CONFIG_FILE) $(ESSENTIALS)
MACROS = __TOOLS
endif

ifeq ($(TYPE), preprocessor)
LDPARAM =
CCPARAM = -nodefaultlibs -mv810 -Wall -O -Winline -std=gnu99 -fstrict-aliasing -include $(CONFIG_FILE) $(ESSENTIALS) -E -P
MACROS = __TOOLS
endif


# Add directories to the include and library paths
INCPATH := $(shell find $(VBJAENGINE) -type d -print)

# Which files to add to backups, apart from the source code
EXTRA_FILES = makefile

# Where to store object and dependency files.
STORE = .make-$(TYPE)-$(COMPILER)-$(COMPILER_OUTPUT)

# Makes a list of the source (.c) files.
SOURCE := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))

# Makes a list of the source (.s) files.
ASM := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.s))

# List of header files.
HEADERS := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.h))

# Makes a list of the object files that will have to be created.
OBJECTS := $(addprefix $(STORE)/, $(SOURCE:.c=.o))

# Makes a list of the object files that will have to be created.
ASM_OBJECTS := $(addprefix $(STORE)/, $(ASM:.s=.o))

# Same for the .d (dependency) files.
DFILES := $(addprefix $(STORE)/,$(SOURCE:.c=.d))

# Specify phony rules. These are rules that are not real files.
.PHONY: clean backup dirs

# Main target. The @ in front of a command prevents make from displaying it to the standard output.

all: $(TARGET).a

$(TARGET).a: dirs $(OBJECTS) $(ASM_OBJECTS)
	@echo Config file: $(CONFIG_FILE)
	@echo Creating $(TARGET).a
	@$(AR) rcs $@ $(ASM_OBJECTS) $(OBJECTS)
	@echo Done creating $@ in $(TYPE) mode with GCC $(COMPILER)

# Rule for creating object file and .d file, the sed magic is to add the object path at the start of the file
# because the files gcc outputs assume it will be in the same dir as the source file.
$(STORE)/%.o: %.c
	@echo Creating object file for $*
	@$(GCC) -Wp,-MD,$(STORE)/$*.dd  $(foreach INC,$(INCPATH),-I$(INC))\
            $(foreach MACRO,$(MACROS),-D$(MACRO)) $(CCPARAM) -$(COMPILER_OUTPUT) $< -o $@
	@sed -e '1s/^\(.*\)$$/$(subst /,\/,$(dir $@))\1/' $(STORE)/$*.dd > $(STORE)/$*.d
	@rm -f $(STORE)/$*.dd

$(STORE)/%.o: %.s
	@echo Creating object file for $*
	@$(AS) -o $@ $<

# Empty rule to prevent problems when a header is deleted.
%.h: ;

# Cleans up the objects, .d files and executables.
clean:
		@echo Making clean.
		@-rm -f $(foreach DIR,$(DIRS),$(STORE)/$(DIR)/*.d $(STORE)/$(DIR)/*.o)
		@-rm -Rf $(STORE)

# Backup the source files.
backup:
		@-if [ ! -e .backup ]; then mkdir .backup; fi;
		@zip .backup/backup_`date +%d-%m-%y_%H.%M`.zip $(SOURCE) $(HEADERS) $(EXTRA_FILES)

# Create necessary directories
dirs:
		@-if [ ! -e $(STORE) ]; then mkdir $(STORE); fi;
		@-$(foreach DIR,$(DIRS), if [ ! -e $(STORE)/$(DIR) ]; \
         then mkdir -p $(STORE)/$(DIR); fi; )

# Includes the .d files so it knows the exact dependencies for every source
-include $(DFILES)
