# Fully qualified plugin name
# (this is set from outside)
NAME = vuengine/plugins/plugin-name

# Clean plugin name by stripping out everything up to (and including) the last slash
BASENAME = $(shell echo $(NAME) | sed -e "s@.*/@@")

# Default build type
TYPE = release
#TYPE = beta
#TYPE = tools
#TYPE = debug
#TYPE = preprocessor

# Default clean type
CLEAN_TYPE = all

# Engine name
ENGINE_NAME = vuengine/core

# Where the game lives
GAME_HOME = .

# Engine's home
VUENGINE_HOME = $(VBDE)libs/$(ENGINE_NAME)

# My home
MY_HOME = $(VBDE)libs/$(NAME)

DEPENDENCIES =

# output dir
BUILD_DIR = $(GAME_HOME)/build

# Where to store object and dependency files.
STORE = $(BUILD_DIR)/$(TYPE)$(STORE_SUFIX)

# Where to preprocess source files
WORKING_FOLDER = $(STORE)

# Add directories to the include and plugin paths
#INCLUDE_PATHS = $(shell find $(WORKING_FOLDER)/objects/vuengine -type d -print)
INCLUDE_PATHS = $(shell find $(WORKING_FOLDER)/objects -type d -print | sed -e 's@'"$(GAME_HOME)"/'@@g')

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

# Include overrides
CONFIG_MAKE_FILE =
ifneq ($(CONFIG_MAKE_FILE),)
include $(CONFIG_MAKE_FILE)
endif

PLUGINS =

# Include custom overrides
ifneq ("$(wildcard $(MY_HOME)/config.make)","")
include $(MY_HOME)/config.make
endif

# Plugins that are linked
ifneq ($(NAME), $(ENGINE_NAME))
PLUGINS := $(ENGINE_NAME) $(PLUGINS)
endif

PLUGIN_MAKEFILE=$(VUENGINE_HOME)/makefile

OPTIMIZATION_OPTION = -O0
ifneq ($(OPTIMIZATION),)
OPTIMIZATION_OPTION = -$(OPTIMIZATION)
endif

PEDANTIC_WARNINGS_FLAG =
ifeq ($(PRINT_PEDANTIC_WARNINGS), 1)
PEDANTIC_WARNINGS_FLAG = -pedantic
endif

STORE_SUFIX =
PROLOG_FUNCTIONS_FLAG =
ifeq ($(USE_PROLOG_FUNCTIONS), 1)
PROLOG_FUNCTIONS_FLAG = -mprolog-function
STORE_SUFIX = -pf
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
SOURCES_DIRS = $(shell find $(MY_HOME)/source $(MY_HOME)/assets -type d -print)
HEADERS_DIRS = $(shell find $(MY_HOME)/source -type d -print)
SOURCES_DIRS_CLEAN = $(shell echo $(SOURCES_DIRS) | sed -e 's@'"$(MY_HOME)"/'@@g')
HEADERS_DIRS_CLEAN = $(shell echo $(HEADERS_DIRS) | sed -e 's@'"$(MY_HOME)"/'@@g')


# Obligatory headers
CONFIG_FILE = 			$(VUENGINE_HOME)/source/config.h
ESSENTIAL_HEADERS = 	-include $(CONFIG_FILE) 																				\
						-include $(VUENGINE_HOME)/source/libvuengine.h 															\
						$(foreach PLUGIN, $(PLUGINS), $(shell if [ -f $(VBDE)libs/$(PLUGIN)/source/config.h ]; then 			\
							echo -include $(VBDE)libs/$(PLUGIN)/source/config.h; fi; ))											\
						$(shell if [ -f $(MY_HOME)/source/config.h ]; then echo -include $(MY_HOME)/source/config.h; fi;)		\
						$(GAME_ESSENTIAL_HEADERS)			

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

ifeq ($(TYPE), preprocessor)
ALL_TARGET_PREREQUISITES = $(C_OBJECTS)
LD_PARAMS =
C_PARAMS = -std=gnu99 -mv810 -nodefaultlibs -Wall -Wextra -E
MACROS = $(COMMON_MACROS)
endif

FOLDER_TO_CLEAN=$(MY_HOME)/$(BUILD_DIR)
ifneq ($(CLEAN_TYPE), all)
FOLDER_TO_CLEAN=$(MY_HOME)/$(BUILD_DIR)/$(CLEAN_TYPE)*
endif

# Makes a list of the source (.c) files.
C_SOURCE = $(foreach DIR,$(SOURCES_DIRS),$(wildcard $(DIR)/*.c))

# Makes a list of the source (.s) files.
ASSEMBLY_SOURCE = $(foreach DIR,$(SOURCES_DIRS),$(wildcard $(DIR)/*.s))

# List of header files.
HEADERS = $(foreach DIR,$(HEADERS_DIRS),$(wildcard $(DIR)/*.h))

# Makes a list of the header files that will have to be created.
H_FILES_TEMP = $(addprefix $(WORKING_FOLDER)/objects/$(NAME)/, $(HEADERS:.h=.h))
H_FILES = $(shell echo $(H_FILES_TEMP) | sed -e 's@'"$(MY_HOME)"/'@@g')

# Makes a list of the object files that will have to be created.
C_OBJECTS_TEMP = $(addprefix $(STORE)/objects/$(NAME)/, $(C_SOURCE:.c=.o))
C_OBJECTS = $(shell echo $(C_OBJECTS_TEMP) | sed -e 's@'"$(MY_HOME)"/'@@g')

C_INTERMEDIATE_SOURCES_TEMP = $(addprefix $(WORKING_FOLDER)/objects/$(NAME)/, $(C_SOURCE:.c=.c))
C_INTERMEDIATE_SOURCES = $(shell echo $(C_INTERMEDIATE_SOURCES_TEMP) | sed -e 's@'"$(MY_HOME)"/'@@g')

# Makes a list of the object files that will have to be created.
ASSEMBLY_OBJECTS_TEMP = $(addprefix $(STORE)/objects/$(NAME)/, $(ASSEMBLY_SOURCE:.s=.o))
ASSEMBLY_OBJECTS = $(shell echo $(ASSEMBLY_OBJECTS_TEMP) | sed -e 's@'"$(MY_HOME)"/'@@g')


HELPERS_PREFIX = $(BASENAME)

# Class setup file
SETUP_CLASSES = $(HELPERS_PREFIX)SetupClasses
SETUP_CLASSES_SOURCE = $(WORKING_FOLDER)/objects/$(NAME)/$(SETUP_CLASSES)
SETUP_CLASSES_OBJECT = $(STORE)/objects/$(NAME)/$(SETUP_CLASSES)

# Same for the .d (dependency) files.
ifneq ($(PREPROCESS), 1)
D_FILES = $(C_OBJECTS:.o=.d)
D_FILES := $(D_FILES) $(STORE)/objects/$(NAME)/$(SETUP_CLASSES).d
D_FILES := $(shell echo $(D_FILES) | sed -e 's@'"$(MY_HOME)"/'@@g')
else
D_FILES = $(shell if [ -d $(WORKING_FOLDER)/classes/dependencies/$(NAME) ]; then find $(WORKING_FOLDER)/classes/dependencies/$(NAME) -name "*.d"; fi; )
endif

# File that holds the classes hierarchy
CLASSES_HIERARCHY_FILE = $(WORKING_FOLDER)/classes/hierarchies/$(NAME)/classesHierarchy.txt

# the target file
TARGET_FILE = lib$(BASENAME)
TARGET = $(STORE)/$(TARGET_FILE)-$(TYPE)

# Main target. The @ in front of a command prevents make from displaying it to the standard output.
all: printPreBuildingInfo preprocessClasses plugins printBuildingInfo $(TARGET).a printPostBuildingInfo

printPreBuildingInfo:

printBuildingInfo:
	@$(eval START_TIME=$(shell date +%s))
	@echo ""
	@echo "********************************************* Building $(BASENAME)"

printPostBuildingInfo:
	@$(eval END_TIME=$(shell date +%s))
	@echo "Total time:" $$(( ($(END_TIME) - $(START_TIME)) / 60 ))" min. "$$(( ($(END_TIME) - $(START_TIME)) % 60 ))" sec."

$(TARGET).a: $(H_FILES) $(C_OBJECTS) $(C_INTERMEDIATE_SOURCES) $(ASSEMBLY_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo -n Linking $(TARGET_FILE)-$(TYPE)...
	@$(AR) rcsT $@ $(foreach PLUGIN, $(PLUGINS), $(STORE)/lib$(shell echo $(PLUGIN)-$(TYPE) | sed -e "s@.*/@@").a) $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo " done"

$(BUILD_DIR)/$(TARGET_FILE).a: plugins printBuildingInfo $(TARGET).a printPostBuildingInfo
	@cp $(TARGET).a $(BUILD_DIR)/$(TARGET_FILE).a

$(SETUP_CLASSES_OBJECT).o: $(SETUP_CLASSES_SOURCE).c
	@bash $(VUENGINE_HOME)/lib/compiler/preprocessor/printCompilingInfo.sh $<
	@$(GCC) -Wp,-MD,$*.dd $(foreach INC,$(INCLUDE_PATHS),-I$(INC))\
        $(foreach MACRO,$(MACROS),-D$(MACRO)) $(C_PARAMS) -$(COMPILER_OUTPUT) $< -o $@
	@sed -e '1s/^\(.*\)$$/$(subst /,\/,$(dir $@))\1/' $*.dd > $*.d
	@rm -f $*.dd
	@echo " done"

$(SETUP_CLASSES_SOURCE).c: $(H_FILES)
	@bash $(VUENGINE_HOME)/lib/compiler/preprocessor/setupClasses.sh -n $(SETUP_CLASSES) -c $(CLASSES_HIERARCHY_FILE) -o $(SETUP_CLASSES_SOURCE).c -w $(WORKING_FOLDER)

preprocessClasses: dirs preprocessPlugins printClassPreprocessingInfo $(H_FILES)
	@touch $(CLASSES_HIERARCHY_FILE)
	@$(eval PREPROCESSING_CLASSES_END_TIME=$(shell date +%s))
	@echo "Total time:" $$(( ($(PREPROCESSING_CLASSES_END_TIME) - $(PREPROCESSING_CLASSES_START_TIME)) / 60 ))" min. "$$(( ($(PREPROCESSING_CLASSES_END_TIME) - $(PREPROCESSING_CLASSES_START_TIME)) % 60 ))" sec."

printClassPreprocessingInfo:
	@echo
	@echo "********************************************* Preprocessing $(BASENAME)"
	@$(eval PREPROCESSING_CLASSES_START_TIME=$(shell date +%s))

preprocessPlugins:
	@-$(foreach PLUGIN, $(PLUGINS),							 																						\
		$(eval PLUGIN_CLASSES_HIERARCHY_FILE=$(WORKING_FOLDER)/classes/hierarchies/$(PLUGIN)/classesHierarchy.txt)									\
		if [ -f $(PLUGIN_CLASSES_HIERARCHY_FILE) ] && [ ! -f $(CLASSES_HIERARCHY_FILE) ]; then continue; fi; 										\
		$(eval MY_CLASSES_HIERARCHY_FILE_IS_NEWER=$(shell find $(CLASSES_HIERARCHY_FILE) -prune -newer $(PLUGIN_CLASSES_HIERARCHY_FILE) 2>&1))		\
		if [ ! -f $(PLUGIN_CLASSES_HIERARCHY_FILE) ] || [ -f $(CLASSES_HIERARCHY_FILE) -a ! -z "$(MY_CLASSES_HIERARCHY_FILE_IS_NEWER)" ]; then 		\
			$(eval CUSTOM_PLUGIN_MAKEFILE=$(VBDE)libs/$(PLUGIN)/makefile)																			\
			$(MAKE) --no-print-directory preprocessClasses																							\
				-f $(shell if [ -f $(CUSTOM_PLUGIN_MAKEFILE) ]; then echo $(CUSTOM_PLUGIN_MAKEFILE); else echo $(PLUGIN_MAKEFILE); fi; )			\
				-e GAME_HOME=$(GAME_HOME)																											\
				-e NAME=$(PLUGIN)																													\
				-e PREPROCESS=1;																													\
		fi;																																			\
	)

plugins:
	@-$(foreach PLUGIN, $(PLUGINS), 	 																											\
		$(eval CUSTOM_PLUGIN_MAKEFILE=$(VBDE)libs/$(PLUGIN)/makefile)																				\
		$(eval PLUGIN_FILE=$(BUILD_DIR)/lib$(shell echo $(PLUGIN) | sed -e "s@.*/@@"))																\
		if [ ! -f $(PLUGIN_FILE).a ]; then 																											\
			$(MAKE) --no-print-directory	 																										\
				$(PLUGIN_FILE).a 																													\
				-f $(shell if [ -f $(CUSTOM_PLUGIN_MAKEFILE) ]; then echo $(CUSTOM_PLUGIN_MAKEFILE); else echo $(PLUGIN_MAKEFILE); fi; )			\
				-e TYPE=$(TYPE) 																													\
				-e CONFIG_FILE=$(CONFIG_FILE) 																										\
				-e CONFIG_MAKE_FILE=$(CONFIG_MAKE_FILE) 																							\
				-e GAME_HOME=$(GAME_HOME)																											\
				-e GAME_ESSENTIAL_HEADERS="$(ESSENTIAL_HEADERS)"																					\
				-e NAME=$(PLUGIN);																													\
		fi;																																			\
	)

# Rule for creating object file and .d file, the sed magic is to add the object path at the start of the file
# because the files gcc outputs assume it will be in the same dir as the source file.
$(STORE)/objects/$(NAME)/%.o: $(WORKING_FOLDER)/objects/$(NAME)/%.c
	@$(GCC) -Wp,-MD,$(STORE)/objects/$(NAME)/$*.dd $(foreach INC,$(INCLUDE_PATHS),-I$(INC))\
        $(foreach MACRO,$(MACROS),-D$(MACRO)) $(C_PARAMS) -$(COMPILER_OUTPUT) $< -o $@ 2>&1 | bash $(VUENGINE_HOME)/lib/compiler/preprocessor/processGCCOutput.sh -w $(WORKING_FOLDER) -np $(VBDE)libs -n $(NAME) -lp $(VBDE)libs -l $(PLUGINS)
	@sed -e '1s/^\(.*\)$$/$(subst /,\/,$(dir $@))\1/' $(STORE)/objects/$(NAME)/$*.dd > $(STORE)/objects/$(NAME)/$*.dd.tmp 
	@sed -e 's#$<##' $(STORE)/objects/$(NAME)/$*.dd.tmp > $(STORE)/objects/$(NAME)/$*.dd
	@sed -e 's#$@#$<#' $(STORE)/objects/$(NAME)/$*.dd > $(STORE)/objects/$(NAME)/$*.d.tmp
	@sed -e '/^[ 	\\]*$$/d' $(STORE)/objects/$(NAME)/$*.d.tmp > $(STORE)/objects/$(NAME)/$*.d
	@rm -f $(STORE)/objects/$(NAME)/$*.dd
	@rm -f $(STORE)/objects/$(NAME)/$*.dd.tmp
	@rm -f $(STORE)/objects/$(NAME)/$*.d.tmp
	@echo " done"

$(WORKING_FOLDER)/objects/$(NAME)/%.c: $(MY_HOME)/%.c
	@bash $(VUENGINE_HOME)/lib/compiler/preprocessor/printCompilingInfo.sh $<
	@bash $(VUENGINE_HOME)/lib/compiler/preprocessor/processSourceFile.sh -i $< -o $@ -d -w $(WORKING_FOLDER) -c $(CLASSES_HIERARCHY_FILE) -p $(ENGINE_NAME) $($(BASENAME)_PLUGINS) $(BASENAME)

$(STORE)/objects/$(NAME)/%.o: $(MY_HOME)/%.s
	@bash $(VUENGINE_HOME)/lib/compiler/preprocessor/printCompilingInfo.sh $<
	@$(AS) -o $@ $<
	@echo " done"

PLUGINS_ARGUMENT="$(addprefix :, $(PLUGINS:.=.))"

$(WORKING_FOLDER)/objects/$(NAME)/%.h: $(MY_HOME)/%.h
	@bash $(VUENGINE_HOME)/lib/compiler/preprocessor/processHeaderFile.sh -i $< -o $@ -w $(WORKING_FOLDER) -c $(CLASSES_HIERARCHY_FILE) -n $(NAME) -h $(MY_HOME) -p $(VBDE)libs -l $(PLUGINS_ARGUMENT)

# Empty rule to prevent problems when a header is deleted.
%.h: ;

# Cleans up the objects, .d files and executables.
clean:
	@echo Cleaning $(CLEAN_TYPE)
	@rm -Rf $(FOLDER_TO_CLEAN)
	@echo "Cleaning done."

# Create necessary directories
DIRS_EXIST=$(shell [ -e $(STORE)/objects/$(NAME) ] && echo 1 || echo 0 )

checkPluginsDirs:
	@-$(foreach PLUGIN, $(PLUGINS), 		 																									\
		$(eval CUSTOM_PLUGIN_MAKEFILE=$(VBDE)libs/$(PLUGIN)/makefile)																			\
		$(eval PLUGIN_FILE=$(BUILD_DIR)/lib$(shell echo $(PLUGIN) | sed -e "s@.*/@@").a )														\
		$(MAKE) --no-print-directory dirs																										\
			-f $(shell if [ -f $(CUSTOM_PLUGIN_MAKEFILE) ]; then echo $(CUSTOM_PLUGIN_MAKEFILE); else echo $(PLUGIN_MAKEFILE); fi; )			\
			-e TYPE=$(TYPE) 																													\
			-e GAME_HOME=$(GAME_HOME)																											\
			-e NAME=$(PLUGIN);																													\
	)

printDirsInfo: phony
ifeq ($(DIRS_EXIST), 0)
	@echo -n Checking working dirs for $(BASENAME)...
endif

dirs: checkPluginsDirs printDirsInfo
ifeq ($(DIRS_EXIST), 0)
	@mkdir -p $(WORKING_FOLDER)/classes/dictionaries
	@mkdir -p $(WORKING_FOLDER)/classes/dependencies/$(NAME)
	@mkdir -p $(WORKING_FOLDER)/classes/hierarchies/$(NAME)
	@-$(foreach DIR,$(SOURCES_DIRS_CLEAN), mkdir -p $(WORKING_FOLDER)/objects/$(NAME)/$(DIR); )
	@echo " done"
endif

phony:
	@echo > /dev/null

# Includes the .d files so it knows the exact dependencies for every source
-include $(D_FILES)
