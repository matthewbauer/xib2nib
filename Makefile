LD ?= c++
CXX ?= c++
PREFIX ?= /usr

LDFLAGS ?= -lpugixml -lPlistCpp -lc
CXXFLAGS ?= -std=c++11

SOURCES = $(wildcard *.cpp)
OBJECTS = $(SOURCES:.cpp=.o)

.PHONY: all
all: xib2nib

xib2nib: $(OBJECTS)
	$(CXX) $(LDFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: install
install: xib2nib
	install -D $^ $(PREFIX)/bin/

.PHONY: clean
clean:
	rm -f $(PROGRAM) $(OBJECTS)
