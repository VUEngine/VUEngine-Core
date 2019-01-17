# Project name
NAME = vuengine/core

# Clean plugin name by stripping out everything up to (and including) the last slash
BASENAME = $(shell echo $(NAME) | sed -e "s@.*/@@")

# Default build type
TYPE = release
#TYPE = beta
#TYPE = tools
#TYPE = debug
#TYPE = preprocessor

# Where the game lives
GAME_HOME = .

# My home
MY_HOME = $(VBDE)libs/$(NAME)

# output dir
BUILD_DIR = $(GAME_HOME)/build

# Where to store object and dependency files.
STORE = $(BUILD_DIR)/$(TYPE)$(STORE_SUFFIX)

# Where to preprocess source files
PREPROCESSOR_WORKING_FOLDER = $(BUILD_DIR)/working

# Add directories to the include and library paths
INCLUDE_PATHS = $(shell find $(PREPROCESSOR_WORKING_FOLDER)/headers -type d -print)

# target's needed steps
ALL_TARGET_PREREQUISITES =  $(TARGET).a

# compiler
COMPILER_VERSION = 4.7
COMPILER_OUTPUT = c
COMPILER_NAME = v810

GCC = $(COMPILER_NAME)-gcc
AS = $(COMPILER_NAME)-as
AR = $(COMPILER_NAME)-ar
OBJCOPY = $(COMPILER_NAME)-objcopy
OBJDUMP = $(COMPILER_NAME)-objdump

# Small data sections' usage
MSDA_SIZE                       = 0
MEMORY_POOL_SECTION             =
NON_INITIALIZED_DATA_SECTION    =
INITIALIZED_DATA_SECTION        =
STATIC_SINGLETONS_DATA_SECTION  =
VIRTUAL_TABLES_DATA_SECTION     =

MEMORY_POOL_SECTION_ATTRIBUTE               = __MEMORY_POOL_SECTION_ATTRIBUTE=
NON_INITIALIZED_DATA_SECTION_ATTRIBUTE      = __NON_INITIALIZED_DATA_SECTION_ATTRIBUTE=
INITIALIZED_DATA_SECTION_ATTRIBUTE          = __INITIALIZED_DATA_SECTION_ATTRIBUTE=
STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE    = __STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE=
VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE       = __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE=

# include overrides
CONFIG_MAKE_FILE =
ifneq ($(CONFIG_MAKE_FILE),)
include $(CONFIG_MAKE_FILE)
endif

OPTIMIZATION_OPTION = -O0
ifneq ($(OPTIMIZATION),)
OPTIMIZATION_OPTION = -$(OPTIMIZATION)
endif

PEDANTIC_WARNINGS_FLAG =
ifeq ($(PRINT_PEDANTIC_WARNINGS), 1)
PEDANTIC_WARNINGS_FLAG = -pedantic
endif

STORE_SUFFIX =
PROLOG_FUNCTIONS_FLAG =
ifeq ($(USE_PROLOG_FUNCTIONS), 1)
PROLOG_FUNCTIONS_FLAG = -mprolog-function
STORE_SUFFIX = -pf
endif

FRAME_POINTER_USAGE_FLAG = -fomit-frame-pointer
ifeq ($(USE_FRAME_POINTER), 1)
FRAME_POINTER_USAGE_FLAG = -fno-omit-frame-pointer
endif


ifneq ($(MEMORY_POOL_SECTION),)
MEMORY_POOL_SECTION_ATTRIBUTE = __MEMORY_POOL_SECTION_ATTRIBUTE="__attribute__((section(\"$(MEMORY_POOL_SECTION)\")))"
endif

ifneq ($(NON_INITIALIZED_DATA_SECTION),)
NON_INITIALIZED_DATA_SECTION_ATTRIBUTE = __NON_INITIALIZED_DATA_SECTION_ATTRIBUTE="__attribute__((section(\"$(NON_INITIALIZED_DATA_SECTION)\")))"
endif

ifneq ($(INITIALIZED_DATA_SECTION),)
INITIALIZED_DATA_SECTION_ATTRIBUTE = __INITIALIZED_DATA_SECTION_ATTRIBUTE="__attribute__((section(\"$(INITIALIZED_DATA_SECTION)\")))"
endif

ifneq ($(STATIC_SINGLETONS_DATA_SECTION),)
STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE = __STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE="__attribute__((section(\"$(STATIC_SINGLETONS_DATA_SECTION)\")))"
endif

ifneq ($(VIRTUAL_TABLES_DATA_SECTION),)
VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE = __VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE="__attribute__((section(\"$(VIRTUAL_TABLES_DATA_SECTION)\")))"
endif

DATA_SECTION_ATTRIBUTES = $(MEMORY_POOL_SECTION_ATTRIBUTE) $(NON_INITIALIZED_DATA_SECTION_ATTRIBUTE) $(INITIALIZED_DATA_SECTION_ATTRIBUTE) $(STATIC_SINGLETONS_DATA_SECTION_ATTRIBUTE) $(VIRTUAL_TABLES_DATA_SECTION_ATTRIBUTE)

# Which directories contain source files
SOURCES_DIRS = $(shell find $(MY_HOME)/source $(MY_HOME)/assets $(MY_HOME)/lib/compiler -type d -print)
HEADERS_DIRS = $(shell find $(MY_HOME)/source -type d -print)

# Obligatory headers
CONFIG_FILE =       $(shell pwd)/source/config.h
ESSENTIAL_HEADERS = -include $(CONFIG_FILE) \
                    -include $(MY_HOME)/source/libvuengine.h \
                    $(foreach PLUGIN, $(PLUGINS), $(shell if [ -f $(VBDE)libs/$(PLUGIN)/source/config.h ]; then echo -include $(VBDE)libs/$(PLUGIN)/source/config.h; fi; )) \

# Common macros for all build types
COMMON_MACROS = $(DATA_SECTION_ATTRIBUTES)

# The following blocks change some variables depending on the build type
ifeq ($(TYPE),debug)
LD_PARAMS = -T$(LINKER_SCRIPT) -lm
C_PARAMS = $(ESSENTIAL_HEADERS) $(PROLOG_FUNCTIONS_FLAG) $(FRAME_POINTER_USAGE_FLAG) $(PEDANTIC_WARNINGS_FLAG) $(OPTIMIZATION_OPTION) -std=gnu99 -mv810 -nodefaultlibs -Wall -Wextra -finline-functions -Winline
MACROS = __DEBUG __TOOLS $(COMMON_MACROS)
endif

ifeq ($(TYPE), release)
LD_PARAMS = -T$(LINKER_SCRIPT) -lm
C_PARAMS = $(ESSENTIAL_HEADERS) $(PROLOG_FUNCTIONS_FLAG) $(FRAME_POINTER_USAGE_FLAG) $(PEDANTIC_WARNINGS_FLAG) $(OPTIMIZATION_OPTION) -std=gnu99 -mv810 -nodefaultlibs -Wall -Wextra -finline-functions -Winline
MACROS = __RELEASE $(COMMON_MACROS)
endif

ifeq ($(TYPE), beta)
LD_PARAMS = -T$(LINKER_SCRIPT) -lm
C_PARAMS = $(ESSENTIAL_HEADERS) $(PROLOG_FUNCTIONS_FLAG) $(FRAME_POINTER_USAGE_FLAG) $(PEDANTIC_WARNINGS_FLAG) $(OPTIMIZATION_OPTION)  -std=gnu99 -mv810 -nodefaultlibs -Wall -Wextra -finline-functions -Winline
MACROS = __BETA $(COMMON_MACROS)
endif

ifeq ($(TYPE), tools)
LD_PARAMS = -T$(LINKER_SCRIPT) -lm
C_PARAMS = $(ESSENTIAL_HEADERS) $(PROLOG_FUNCTIONS_FLAG) $(FRAME_POINTER_USAGE_FLAG) $(PEDANTIC_WARNINGS_FLAG) $(OPTIMIZATION_OPTION)  -std=gnu99 -mv810 -nodefaultlibs -Wall -Wextra -finline-functions -Winline
MACROS = __TOOLS $(COMMON_MACROS)
endif

ifeq ($(TYPE),preprocessor)
ALL_TARGET_PREREQUISITES = $(C_OBJECTS)
LD_PARAMS =
C_PARAMS = -std=gnu99 -mv810 -nodefaultlibs -Wall -Wextra -E
MACROS = $(COMMON_MACROS)
endif

# Makes a list of the source (.c) files.
C_SOURCE = $(foreach DIR,$(SOURCES_DIRS),$(wildcard $(DIR)/*.c))

# Makes a list of the source (.s) files.
ASSEMBLY_SOURCE = $(foreach DIR,$(SOURCES_DIRS),$(wildcard $(DIR)/*.s))

# List of header files.
HEADERS = $(foreach DIR,$(HEADERS_DIRS),$(wildcard $(DIR)/*.h))

# Makes a list of the header files that will have to be created.
H_FILES = $(addprefix $(PREPROCESSOR_WORKING_FOLDER)/headers/$(NAME)/, $(HEADERS:.h=.h))

# Makes a list of the object files that will have to be created.
C_OBJECTS = $(addprefix $(STORE)/objects/$(NAME)/, $(C_SOURCE:.c=.o))
C_INTERMEDIATE_SOURCES = $(addprefix $(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME)/, $(C_SOURCE:.c=.c))

# Makes a list of the object files that will have to be created.
ASSEMBLY_OBJECTS = $(addprefix $(STORE)/objects/$(NAME)/, $(ASSEMBLY_SOURCE:.s=.o))

HELPERS_PREFIX = $(BASENAME)

# Class setup file
SETUP_CLASSES = $(HELPERS_PREFIX)SetupClasses
SETUP_CLASSES_OBJECT = $(STORE)/objects/$(NAME)/$(SETUP_CLASSES)

# Same for the .d (dependency) files.
D_FILES = $(addprefix $(STORE)/objects/$(NAME)/,$(C_SOURCE:.c=.d))
D_FILES := $(D_FILES) $(STORE)/objects/$(NAME)/$(SETUP_CLASSES).d

# File that holds the classes hierarchy
CLASSES_HIERARCHY_FILE=$(PREPROCESSOR_WORKING_FOLDER)/$(HELPERS_PREFIX)ClassesHierarchy.txt

# the target file
TARGET_FILE = lib$(BASENAME)
TARGET = $(STORE)/$(TARGET_FILE)-$(TYPE)

# Main target. The @ in front of a command prevents make from displaying it to the standard output.
all: printBuildingInfo dirs $(TARGET).a

printBuildingInfo:
	@echo Building $(TARGET_FILE).a
#	@echo Build type: $(TYPE)
#	@echo Compiler: $(COMPILER_NAME) $(COMPILER_VERSION)
#	@echo Compiler\'s output: $(COMPILER_OUTPUT)

$(TARGET).a: $(H_FILES) $(C_OBJECTS) $(C_INTERMEDIATE_SOURCES) $(SETUP_CLASSES_OBJECT).o $(ASSEMBLY_OBJECTS)
	@echo Linking $(TARGET_FILE).a
	@$(AR) rcs $@ $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o

$(BUILD_DIR)/$(TARGET_FILE).a: $(TARGET).a
	@cp $(TARGET).a $(BUILD_DIR)/$(TARGET_FILE).a

$(SETUP_CLASSES_OBJECT).o: $(PREPROCESSOR_WORKING_FOLDER)/$(SETUP_CLASSES).c
	@echo $< | sed -e 's#'"$(PREPROCESSOR_WORKING_FOLDER)"/sources/$(NAME)/'#Compiling #g'
	@$(GCC) -Wp,-MD,$*.dd $(foreach INC,$(INCLUDE_PATHS),-I$(INC))\
        $(foreach MACRO,$(MACROS),-D$(MACRO)) $(C_PARAMS) -$(COMPILER_OUTPUT) $< -o $@
	@sed -e '1s/^\(.*\)$$/$(subst /,\/,$(dir $@))\1/' $*.dd > $*.d
	@rm -f $*.dd

$(PREPROCESSOR_WORKING_FOLDER)/$(SETUP_CLASSES).c: $(H_FILES)
	@bash $(MY_HOME)/lib/compiler/preprocessor/setupClasses.sh -c $(CLASSES_HIERARCHY_FILE) -o $(SETUP_CLASSES).c -w $(PREPROCESSOR_WORKING_FOLDER)

# Rule for creating object file and .d file, the sed magic is to add the object path at the start of the file
# because the files gcc outputs assume it will be in the same dir as the source file.
$(STORE)/objects/$(NAME)/%.o: $(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME)/%.c
	@echo $< | sed -e 's#'"$(PREPROCESSOR_WORKING_FOLDER)"/sources/$(NAME)/'#Compiling #g'
	@$(GCC) -Wp,-MD,$(STORE)/objects/$(NAME)/$*.dd $(foreach INC,$(INCLUDE_PATHS),-I$(INC))\
        $(foreach MACRO,$(MACROS),-D$(MACRO)) $(C_PARAMS) -$(COMPILER_OUTPUT) $< -o $@
	@sed -e '1s/^\(.*\)$$/$(subst /,\/,$(dir $@))\1/' $(STORE)/objects/$(NAME)/$*.dd > $(STORE)/objects/$(NAME)/$*.d
	@rm -f $(STORE)/objects/$(NAME)/$*.dd

$(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME)/%.c: %.c
	@bash $(MY_HOME)/lib/compiler/preprocessor/processSourceFile.sh -i $< -o $@ -d -w $(PREPROCESSOR_WORKING_FOLDER) -c $(CLASSES_HIERARCHY_FILE) -p $(NAME)

$(STORE)/objects/$(NAME)/%.o: %.s
	@echo Creating object file for $*
	@$(AS) -o $@ $<

$(PREPROCESSOR_WORKING_FOLDER)/headers/$(NAME)/%.h: %.h
	@echo Preprocessing $<
	@bash $(MY_HOME)/lib/compiler/preprocessor/processHeaderFile.sh -i $< -o $@ -w $(PREPROCESSOR_WORKING_FOLDER) -c $(CLASSES_HIERARCHY_FILE) -p $(BASENAME)

# Empty rule to prevent problems when a header is deleted.
%.h: ;

# Cleans up the objects, .d files and executables.
clean:
	@echo Cleaning $(TYPE)...$(STORE)
	@rm -Rf $(STORE)
	@echo Cleaning done.

# Create necessary directories
dirs:
	@echo Checking working dirs...
	@-$(foreach DIR,$(HEADERS_DIRS), if [ ! -e $(PREPROCESSOR_WORKING_FOLDER)/headers/$(NAME)/$(DIR) ]; \
         then mkdir -p $(PREPROCESSOR_WORKING_FOLDER)/headers/$(NAME)/$(DIR); fi; )
	@-if [ ! -e $(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME) ]; then mkdir -p $(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME); fi;
	@-$(foreach DIR,$(SOURCES_DIRS), if [ ! -e $(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME)/$(DIR) ]; \
         then mkdir -p $(PREPROCESSOR_WORKING_FOLDER)/sources/$(NAME)/$(DIR); fi; )
	@-if [ ! -e $(STORE)/objects/$(NAME) ]; then mkdir -p $(STORE)/objects/$(NAME); fi;
	@-$(foreach DIR,$(SOURCES_DIRS), if [ ! -e $(STORE)/objects/$(NAME)/$(DIR) ]; \
         then mkdir -p $(STORE)/objects/$(NAME)/$(DIR); fi; )

# Includes the .d files so it knows the exact dependencies for every source
-include $(D_FILES)
