#Makefile taken from Wikipedia.org
#
# Specify the main target
TARGET = libvbjae

# Default build type
#TYPE = debug
TYPE = release
#TYPE = preprocessor

VBJAENGINE = $(VBDE)/libs/vbjaengine

# Which directories contain source files
DIRS := $(shell find $(VBJAENGINE)/source -type d -print)
		
# Which libraries are linked
LIBS =
ROMHEADER = lib/vb.hdr 
# Dynamic libraries
DLIBS =

# Obligatory headers
CONFIG_FILE = $(VBJAENGINE)/config.h

ESSENTIALS =  -include $(VBJAENGINE)/source/base/libgccvb/Libgccvb.h			\
					-include $(VBJAENGINE)/source/base/Constants.h	 				\
					-include $(VBJAENGINE)/source/base/VirtualList.h	 				\
					-include $(VBJAENGINE)/source/hardware/HardwareManager.h		\
					-include $(VBJAENGINE)/source/base/Error.h 						\
					-include $(VBJAENGINE)/source/base/MemoryPool.h 				\
					-include $(VBJAENGINE)/source/graphics/2d/Printing.h

# The next blocks change some variables depending on the build type
ifeq ($(TYPE), debug)
LDPARAM = -fno-builtin -ffreestanding  
CCPARAM = -nodefaultlibs -mv810 -Wall -O -Winline -include $(CONFIG_FILE) $(ESSENTIALS) 
MACROS = __DEBUG
endif

ifeq ($(TYPE), release)
LDPARAM =  
CCPARAM = -nodefaultlibs -mv810 -Wall -O3 -Winline -include $(CONFIG_FILE) $(ESSENTIALS)
MACROS = NDEBUG
endif


ifeq ($(TYPE), preprocessor)
LDPARAM =  
CCPARAM = -nodefaultlibs -mv810 -Wall -O -Winline -include $(CONFIG_FILE) $(ESSENTIALS) -E -P
MACROS = __DEBUG
endif


# Add directories to the include and library paths

INCPATH := $(shell find $(VBJAENGINE) -type d -print)
						 

# Which files to add to backups, apart from the source code
EXTRA_FILES = makefile

# The compiler
GCC = v810-gcc
OBJCOPY = v810-objcopy
OBJDUMP = v810-objdump
AR = v810-ar

# Where to store object and dependancy files.
STORE = .make-$(TYPE)

# Makes a list of the source (.cpp) files.
SOURCE := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.c))

# List of header files.
HEADERS := $(foreach DIR,$(DIRS),$(wildcard $(DIR)/*.h))

# Makes a list of the object files that will have to be created.
OBJECTS := $(addprefix $(STORE)/, $(SOURCE:.c=.o))

# Same for the .d (dependancy) files.
DFILES := $(addprefix $(STORE)/,$(SOURCE:.c=.d))

# Specify phony rules. These are rules that are not real files.
.PHONY: clean backup dirs

# Main target. The @ in front of a command prevents make from displaying
# it to the standard output.

all: $(TARGET).a

$(TARGET).a: dirs $(OBJECTS)
	@echo Config file: $(CONFIG_FILE)
	@echo Creating $(TARGET).a
	@$(AR) rcs $@ $(OBJECTS) 
	@echo Done $@

# Rule for creating object file and .d file, the sed magic is to add
# the object path at the start of the file because the files gcc
# outputs assume it will be in the same dir as the source file.
$(STORE)/%.o: %.c
	@echo Creating o file for $(TYPE) $*...
	@$(GCC) -Wp,-MD,$(STORE)/$*.dd  $(foreach INC,$(INCPATH),-I$(INC))\
            $(foreach MACRO,$(MACROS),-D$(MACRO)) $(CCPARAM) -c $< -o $@
	@sed -e '1s/^\(.*\)$$/$(subst /,\/,$(dir $@))\1/' $(STORE)/$*.dd > $(STORE)/$*.d
	@rm -f $(STORE)/$*.dd

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

# Includes the .d files so it knows the exact dependencies for every
# source.
-include $(DFILES)
