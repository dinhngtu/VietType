CC=gcc-10
CXX=g++-10
CPPFLAGS+=-D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -MMD -MP -include salieri.h -include define.h
CPPFLAGS+=-iquote Telex/ -iquote TestLib/ -iquote WordLister/
COMPFLAGS=-Wall -Wextra -Wformat=2 -Wimplicit-fallthrough=5 -Wshadow -Werror=return-type -fwrapv
CFLAGS+=$(COMPFLAGS) -std=c17
CXXFLAGS+=$(COMPFLAGS) -std=c++20

ifeq ($(DEBUG),y)
CPPFLAGS+=-D_DEBUG
COMPFLAGS+=-g3
else
CPPFLAGS+=-DNDEBUG
COMPFLAGS+=-O2
LDFLAGS+=-s
endif

TARGETS=wordlister
OBJECTS_WORDLISTER=WordLister/WordLister.o WordLister/Bench.o WordLister/DualScan.o WordLister/EngScan.o WordLister/VietScan.o TestLib/FileUtil.o Telex/TelexEngine.o
OBJECTS=$(OBJECTS_WORDLISTER)

all: $(TARGETS)

wordlister: $(OBJECTS_WORDLISTER)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

.PHONY: clean

clean:
	$(RM) $(TARGETS) $(OBJECTS)
