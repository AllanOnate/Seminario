CXX ?= g++
CXXFLAGS ?= -std=gnu++17 -O2 -Wall

CADMIUM_V2_INCLUDE ?= ../cadmium_v2/include
INCLUDES = -I. -I$(CADMIUM_V2_INCLUDE)

BIN_DIR = bin
BUILD_DIR = build

MAIN_BIN = $(BIN_DIR)/CESFAM_V2
MAIN_OBJ = $(BUILD_DIR)/main.o

TEST_GEN_BIN = $(BIN_DIR)/test_generator_v2
TEST_GESTOR_BIN = $(BIN_DIR)/test_gestor_v2
TEST_MEDICO_BIN = $(BIN_DIR)/test_medico_v2

TEST_GEN_OBJ = $(BUILD_DIR)/test_generator.o
TEST_GESTOR_OBJ = $(BUILD_DIR)/test_gestor.o
TEST_MEDICO_OBJ = $(BUILD_DIR)/test_medico.o

.PHONY: all clean dirs test

all: dirs $(MAIN_BIN)

dirs:
	mkdir -p $(BIN_DIR)
	mkdir -p $(BUILD_DIR)
	mkdir -p simulation_results

# ---------------- main ----------------
$(MAIN_OBJ): top_model/main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(MAIN_BIN): $(MAIN_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

# ---------------- tests ----------------
test: dirs $(TEST_GEN_BIN) $(TEST_GESTOR_BIN) $(TEST_MEDICO_BIN)

$(TEST_GEN_OBJ): test/main_generator.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TEST_GEN_BIN): $(TEST_GEN_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(TEST_GESTOR_OBJ): test/main_gestor.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TEST_GESTOR_BIN): $(TEST_GESTOR_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

$(TEST_MEDICO_OBJ): test/main_medico.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(TEST_MEDICO_BIN): $(TEST_MEDICO_OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $^ -o $@

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR) simulation_results/*.csv
