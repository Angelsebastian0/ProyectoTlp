CXX := g++
CXXFLAGS := -std=c++17 -Wall -Ithird_party -Isrc
SRCDIR := src
BINDIR := bin
TARGET := $(BINDIR)/motor_integration

.PHONY: all clean dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BINDIR)

# Compilación directa sin objetos (más simple)
$(TARGET): $(SRCDIR)/integration_main.cpp $(SRCDIR)/engine/api.cpp $(SRCDIR)/interpreter/script_interpreter.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(shell pkg-config --cflags --libs sdl2)

clean:
	rm -rf $(BINDIR)
