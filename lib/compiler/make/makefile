# Fully qualified plugin name
# (this is set from outside)
NAME = context//plugin-name

# Engine name
override ENGINE_NAME = core

# Engine's home
override ENGINE_HOME = $(ENGINE_FOLDER)

# Home of custom user plugins
USER_PLUGINS_FOLDER = $(PLUGINS_FOLDER)

# Clean plugin name by stripping out everything up to (and including) the last slash
CLEAN_NAME = $(shell echo $(NAME) | sed -e "s@.*//@@")

# My home
ifeq ($(NAME), $(ENGINE_NAME))
    override MY_HOME = $(ENGINE_HOME)
else
ifneq (,$(findstring user,$(NAME)))
    override MY_HOME = $(USER_PLUGINS_FOLDER)/$(CLEAN_NAME)
else
    override MY_HOME = $(PLUGINS_FOLDER)/$(CLEAN_NAME)
endif
endif

# Where the game lives
GAME_HOME = .

# Common
include $(ENGINE_HOME)/lib/compiler/make/makefile-common

# the target file
TARGET_FILE = lib$(BASENAME)
TARGET = $(WORKING_FOLDER)/libraries/$(BUILD_MODE)/$(TARGET_FILE)-$(TYPE)

# Main target. The @ in front of a command prevents make from displaying it to the standard output.
all: printPreBuildingInfo preprocessClasses plugins printBuildingInfo $(TARGET).a

printPreBuildingInfo:

printBuildingInfo:
	@echo ""
	@$(shell echo $(NAME) >> $(WORKING_FOLDER)/traces/$(BUILD_MODE)/builtComponents.txt)
#	@$(eval BUILT_COMPONENTS=$(shell wc -l < $(WORKING_FOLDER)/traces/$(BUILD_MODE)/builtComponents.txt))
	@echo "Building $(BASENAME)"
#	@echo "($(BUILT_COMPONENTS)/$(COMPONENTS)) Building $(BASENAME)"
#	@$(eval START_TIME=$(shell date +%s))

printPostBuildingInfo:
	@$(eval END_TIME=$(shell date +%s))
	@echo "Total time:" $$(( ($(END_TIME) - $(START_TIME)) / 60 ))" min. "$$(( ($(END_TIME) - $(START_TIME)) % 60 ))" sec."

$(TARGET).a: $(H_FILES) $(ASSEMBLY_OBJECTS) $(C_OBJECTS) $(SETUP_CLASSES_OBJECT).o
	@echo 
	@echo "Linking $(TARGET_FILE)-$(TYPE)"
	@$(eval SOURCES_OBJECT_FILES=$(shell ls $(WORKING_FOLDER)/objects/$(BUILD_MODE)/hashes/$(NAME)/*.o 2>/dev/null))
	@$(eval ASSETS_OBJECT_FILES=$(shell ls $(WORKING_FOLDER)/assets/$(NAME)/hashes/*.o 2>/dev/null))
	@$(AR) rcsT $@ $(SOURCES_OBJECT_FILES) $(ASSETS_OBJECT_FILES) $(BINARY_ASSETS) $(foreach PLUGIN, $(PLUGINS), $(WORKING_FOLDER)/libraries/$(BUILD_MODE)/lib$(shell echo $(PLUGIN)-$(TYPE) | sed -e "s@.*/@@").a) 

$(BUILD_DIR)/$(TARGET_FILE).a: plugins printBuildingInfo compile $(TARGET).a
	@cp $(TARGET).a $(BUILD_DIR)/$(TARGET_FILE).a
