# -------- User-configurable --------
TARGET       ?= mandelbrot.exe

# Raylib paths (if your layout is different, adjust these paths)
RAYLIB_DIR   ?= ./thirdparty/raylib-5.0/src

# Build type: "release" (default) or "debug"
BUILD        ?= release

# -------- Tools --------
# Compiler / linker --> adjust if you don't want a C++ compiler
CXX          := g++
AR           := ar

# -------- Windows vs POSIX shell helpers --------
# If make is called from a Windows shell, use cmd.exe and Windows commands. Otherwise, use POSIX shell and commands.
# Powershell has aliases for mkdir and rm, but without the -p and -rf flags, so we have to use cmd.exe instead.
ifeq ($(OS),Windows_NT)
  SHELL        := cmd.exe
  .SHELLFLAGS  := /C
  # Directory/file ops for Windows shell
  define MKDIR
    if not exist "$(1)" mkdir "$(1)"
  endef
  RMFILE       := del /Q /F
  RMDIR        := rmdir /S /Q
  RUN_PREFIX   :=
else
  define MKDIR
    mkdir -p "$(1)"
  endef
  RMFILE       := rm -f
  RMDIR        := rm -rf
  RUN_PREFIX   := ./
endif

# -------- Sources / Outputs --------
SRCS         := $(wildcard *.cpp)
BUILD_DIR    := build/$(BUILD)
OBJS         := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))
DEPS         := $(OBJS:.o=.d)
OUT          := $(BUILD_DIR)/$(TARGET)

# -------- Flags --------
# Common flags
CXXFLAGS     := -std=c++17 -Wall -I$(RAYLIB_DIR)
LDFLAGS      := -L$(RAYLIB_DIR)
LDLIBS       := -lraylib -lgdi32 -lwinmm -lopengl32 -lpthread

# Per-configuration flags
ifeq ($(BUILD),debug)
  CXXFLAGS   += -O0 -g -DDEBUG
else ifeq ($(BUILD),release)
  CXXFLAGS   += -O3 -DNDEBUG
else
  $(error Unknown BUILD=$(BUILD). Use BUILD=debug or BUILD=release)
endif

# The executable will be statically linked by default to be more portable. If you want a dynamically linked executable, comment out the following line.
LDFLAGS += -static -static-libgcc -static-libstdc++

# -------- Default target --------
.PHONY: all # make targets files by default, so we declare "all" as a phony target to avoid conflicts with a file named "all"
all: $(OUT)

# Link into build directory
$(OUT): $(OBJS) | $(BUILD_DIR)
	$(CXX) $(OBJS) $(LDFLAGS) $(LDLIBS) -o $@

# Compile (with depfiles)
$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Create build directory
$(BUILD_DIR):
	$(call MKDIR,$@)

# Include auto-generated dependency files
-include $(DEPS)

# Convenience targets
.PHONY: clean debug release run where
clean:
	-$(RMDIR) build

# Shortcut wrappers
debug:
	$(MAKE) BUILD=debug
release:
	$(MAKE) BUILD=release

# Build then run
run: all
	"$(OUT)"

# Show where the output binary is
where:
	@echo $(OUT)