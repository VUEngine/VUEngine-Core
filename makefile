# Fully qualified plugin name
# (this is set from outside)
NAME = vuengine/plugins/plugin-name

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

# Engine's home
ENGINE_HOME = $(VBDE)libs/$(ENGINE_NAME)

# My home
MY_HOME = $(VBDE)libs/$(NAME)

# Where the game lives
GAME_HOME = .

# Prefix used to clean the include paths
INCLUDE_PATH_CLEAN_PREFIX = $(GAME_HOME)

# output dir
BUILD_DIR = $(GAME_HOME)/build

# target's needed steps
ALL_TARGET_PREREQUISITES =  $(TARGET).a

# Overrides file
PLUGIN_CONFIG_MAKE_FILE = $(shell if [ -f $(MY_HOME)/config.make ]; then echo $(MY_HOME)/config.make; fi;)

# Common
include $(ENGINE_HOME)/makefile-common

# Override plugins
PLUGINS =
-include $(PLUGIN_CONFIG_MAKE_FILE)

ifneq ($(NAME), $(ENGINE_NAME))
PLUGINS := $(ENGINE_NAME) $(PLUGINS)
endif

# Same for the .d (dependency) files.
ifneq ($(PREPROCESS), 1)
D_FILES = $(C_OBJECTS:.o=.d)
D_FILES := $(D_FILES) $(STORE)/objects/$(NAME)/$(SETUP_CLASSES).d
D_FILES := $(shell echo $(D_FILES) | sed -e 's@'"$(MY_HOME)"/'@@g')
else
D_FILES = $(shell if [ -d $(WORKING_FOLDER)/classes/dependencies/$(NAME) ]; then find $(WORKING_FOLDER)/classes/dependencies/$(NAME) -name "*.d"; fi; )
endif

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

$(TARGET).a: $(H_FILES) compile
	@echo -n Linking $(TARGET_FILE)-$(TYPE)...
	@$(AR) rcsT $@ $(foreach PLUGIN, $(PLUGINS), $(STORE)/lib$(shell echo $(PLUGIN)-$(TYPE) | sed -e "s@.*/@@").a) $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo " done"

$(BUILD_DIR)/$(TARGET_FILE).a: plugins printBuildingInfo $(TARGET).a printPostBuildingInfo
	@cp $(TARGET).a $(BUILD_DIR)/$(TARGET_FILE).a
