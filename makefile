# Fully qualified plugin name
# (this is set from outside)
NAME = vuengine/plugin-name

# Engine name
override ENGINE_NAME = core

# Engine's home
override ENGINE_HOME = $(ENGINE_FOLDER)

# TODO: USER_PLUGINS_FOLDER must be given by the invoker of make
USER_PLUGINS_FOLDER = $(ENGINE_HOME)../../custom

# Clean plugin name by stripping out everything up to (and including) the last slash
CLEAN_NAME = $(shell echo $(NAME) | sed -e "s@.*//@@")

# My home
ifeq ($(NAME), $(ENGINE_NAME))
    override MY_HOME = $(ENGINE_HOME)
else
ifneq (,$(findstring vuengine,$(NAME)))
    override MY_HOME = $(PLUGINS_FOLDER)$(CLEAN_NAME)
else
    override MY_HOME = $(USER_PLUGINS_FOLDER)/$(CLEAN_NAME)
endif
endif

# Where the game lives
GAME_HOME = .

# Common
include $(ENGINE_HOME)/makefile-common

# the target file
TARGET_FILE = lib$(BASENAME)
TARGET = $(WORKING_FOLDER)/$(TARGET_FILE)-$(TYPE)

# Main target. The @ in front of a command prevents make from displaying it to the standard output.
all: printPreBuildingInfo preprocessClasses plugins printBuildingInfo $(TARGET).a

printPreBuildingInfo:

printBuildingInfo:
	@echo ""
	@$(shell echo $(NAME) >> \$(WORKING_FOLDER)/traces/builtComponents.txt)
	@$(eval BUILT_COMPONENTS=$(shell wc -l < \$(WORKING_FOLDER)/traces/builtComponents.txt))
	@echo "($(BUILT_COMPONENTS)/$(COMPONENTS)) Building $(BASENAME)"
#	@$(eval START_TIME=$(shell date +%s))

printPostBuildingInfo:
	@$(eval END_TIME=$(shell date +%s))
	@echo "Total time:" $$(( ($(END_TIME) - $(START_TIME)) / 60 ))" min. "$$(( ($(END_TIME) - $(START_TIME)) % 60 ))" sec."

$(TARGET).a: $(H_FILES) $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo 
	@echo "Linking $(TARGET_FILE)-$(TYPE)"
	@$(AR) rcsT $@ $(foreach PLUGIN, $(PLUGINS), $(WORKING_FOLDER)/lib$(shell echo $(PLUGIN)-$(TYPE) | sed -e "s@.*/@@").a) $(ASSEMBLY_OBJECTS) $(WORKING_FOLDER)/objects/hashes/$(NAME)/*.o $(SETUP_CLASSES_OBJECT).o

$(BUILD_DIR)/$(TARGET_FILE).a: plugins printBuildingInfo compile $(TARGET).a
	@cp $(TARGET).a $(BUILD_DIR)/$(TARGET_FILE).a
