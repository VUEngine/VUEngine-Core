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

# the target file
TARGET_FILE = lib$(BASENAME)
TARGET = $(WORKING_FOLDER)/$(TARGET_FILE)-$(TYPE)

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

$(TARGET).a: $(H_FILES) $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo -n Linking $(TARGET_FILE)-$(TYPE)...
	@$(AR) rcsT $@ $(foreach PLUGIN, $(PLUGINS), $(WORKING_FOLDER)/lib$(shell echo $(PLUGIN)-$(TYPE) | sed -e "s@.*/@@").a) $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo " done"

$(BUILD_DIR)/$(TARGET_FILE).a: plugins printBuildingInfo compile $(TARGET).a printPostBuildingInfo
	@cp $(TARGET).a $(BUILD_DIR)/$(TARGET_FILE).a
