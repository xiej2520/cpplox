# https://stackoverflow.com/a/30602701
SRC_DIR := ./src
OBJ_DIR := ./obj
BIN_DIR := ./bin

EXE := $(BIN_DIR)/treelox

SRC := $(wildcard $(SRC_DIR)/*.cpp)
OBJ := $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

CPPFLAGS := -Iinclude -MMD -MP
CXXFLAGS := -std=c++20 -Wall
LDFLAGS := -Llib
#LDLIBS := # no libs

.PHONY: all clean debug

all: $(EXE)

$(EXE): $(OBJ) | $(BIN_DIR)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(BIN_DIR) $(OBJ_DIR):
	mkdir -p $@

clean:
	@$(RM) -rv $(BIN_DIR) $(OBJ_DIR)

debug: CXXFLAGS += -g -DEBUG
debug: all

-include $(OBJ:.o=.d)
