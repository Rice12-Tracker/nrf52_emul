# Compilation flags
CPPFLAGS += $(CFLAGS)  # if you have any C++ specific flags, add them here

# Linker flags
LDLIBS += $(shell pkg-config --libs glib-2.0) -lpthread -lm -lunicorn -ltinyxml2

# Object files
OBJS = Emulator.o Peripheral.o main.o

# Output Binary Name
OUTBIN = main

.PHONY: all clean

all: $(OUTBIN)

$(OUTBIN): $(OBJS)
	$(CXX) $(LDFLAGS) $^ $(LDLIBS) -o $@

clean:
	rm -f $(OUTBIN) $(OBJS)