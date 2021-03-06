# Taken from https://spin.atomicobject.com/2016/08/26/makefile-c-projects/.

BUILD_DIR ?= ./build
SRC_DIR ?= ./src
PROTO_DIR ?= ../common/src
TARGETS_DIR ?= ./src/targets

SRCS := $(shell find $(SRC_DIR) -name '*.cpp' -not -path '$(TARGETS_DIR)/**')
HEADERS := $(shell find $(SRC_DIR) -name '*.h')
PROTOS := $(shell find $(PROTO_DIR) -name '*.proto')
PROTO_SERVICES := $(shell find $(PROTO_DIR)/services -name '*.proto')
PROTO_HEADERS := $(PROTOS:$(PROTO_DIR)/%.proto=$(BUILD_DIR)/gen/%.pb.h) $(PROTO_SERVICES:$(PROTO_DIR)/%.proto=$(BUILD_DIR)/gen/%.grpc.pb.h)
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o) $(PROTOS:$(PROTO_DIR)/%.proto=$(BUILD_DIR)/gen/%.pb.cc.o) $(PROTO_SERVICES:$(PROTO_DIR)/%.proto=$(BUILD_DIR)/gen/%.grpc.pb.cc.o)
DEPS := $(OBJS:.o=.d)
TARGET_SRCS := $(shell find $(TARGETS_DIR) -name *.cpp)
TARGET_OBJS := $(TARGET_SRCS:%=$(BUILD_DIR)/%.o)
TARGET_EXECS := $(TARGET_SRCS:$(TARGETS_DIR)/%.cpp=$(BUILD_DIR)/%)
GRPC_CPP_PLUGIN := `which grpc_cpp_plugin`

INC_DIRS := $(SRC_DIR) $(BUILD_DIR)/gen
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CXX := g++
CPP_FLAGS := $(INC_FLAGS) -MMD -MP -std=c++1z -Wall -O2
LD_LIBS := protobuf grpc grpc++
LD_FLAGS := -L/usr/local/lib -L/usr/lib $(addprefix -l,$(LD_LIBS))

all: $(TARGET_EXECS)

$(TARGET_EXECS): $(BUILD_DIR)/%: $(BUILD_DIR)/$(TARGETS_DIR)/%.cpp.o $(OBJS)
	$(CXX) $^ -o $@ $(LD_FLAGS)

$(BUILD_DIR)/gen/%.pb.cc $(BUILD_DIR)/gen/%.pb.h: $(PROTO_DIR)/%.proto
	$(MKDIR_P) $(dir $@)
	protoc -I$(PROTO_DIR) --cpp_out=$(BUILD_DIR)/gen $<

$(BUILD_DIR)/gen/services/%.grpc.pb.cc $(BUILD_DIR)/gen/services/%.grpc.pb.h: $(PROTO_DIR)/services/%.proto
	$(MKDIR_P) $(dir $@)
	protoc -I$(PROTO_DIR) --grpc_out=$(BUILD_DIR)/gen --plugin=protoc-gen-grpc=$(GRPC_CPP_PLUGIN) $<

$(BUILD_DIR)/%.cpp.o: %.cpp | generated_headers
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPP_FLAGS) -c $< -o $@

$(BUILD_DIR)/gen/%.pb.cc.o: $(BUILD_DIR)/gen/%.pb.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPP_FLAGS) -c $< -o $@

$(BUILD_DIR)/gen/%.grpc.pb.cc.o: $(BUILD_DIR)/gen/%.grpc.pb.cc
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPP_FLAGS) -c $< -o $@

generated_headers: $(PROTO_HEADERS)

.PHONY: generated_headers clean test_ai game_server format

.PRECIOUS: %.pb.h %.pb.cc

test_ais: $(BUILD_DIR)/test_ais

game_server: $(BUILD_DIR)/game_server

format:
	clang-format -i -style=file $(SRCS) $(TARGET_SRCS) $(HEADERS)

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p
