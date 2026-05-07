# Makefile for DeepSeek Balance Plugin
# Usage:
#   make          - Build with MinGW-w64 (native, requires MSYS2/MinGW on PATH)
#   make CROSS=1  - Cross-compile from Linux (uses x86_64-w64-mingw32- prefix)
#   make MSVC=1   - Build with MSVC Build Tools (requires cl.exe on PATH)

TARGET  = DeepSeekBalancePlugin.dll
CC      ?= g++
WINDRES ?= windres
AR      ?= ar

# Detect cross-compilation
ifdef CROSS
    CC      = x86_64-w64-mingw32-g++
    WINDRES = x86_64-w64-mingw32-windres
    AR      = x86_64-w64-mingw32-ar
endif

# Source files
SRCS    = dllmain.cpp DataManager.cpp DeepSeekBalanceItem.cpp \
          DeepSeekBalancePlugin.cpp OptionsDlg.cpp
OBJS    = $(SRCS:.cpp=.o)

# Resource object: MSVC uses .res, MinGW/GCC uses .o
ifdef MSVC
RES_OBJ = DeepSeekBalancePlugin.res
else
RES_OBJ = DeepSeekBalancePlugin.o
endif

# Compiler flags
CXXFLAGS = -O2 -std=c++17 -DBUILD_DLL -finput-charset=utf-8
CXXFLAGS += -D_WIN32_WINNT=0x0600 -DWIN32_LEAN_AND_MEAN
CXXFLAGS += -Wall -Wextra -Wno-unused-parameter
CXXFLAGS += -static-libgcc -static-libstdc++

# Linker flags
LDFLAGS  = -shared -Wl,--out-implib,$(TARGET:.dll=.a)
LDLIBS   = -lwinhttp -lcomctl32 -lkernel32 -luser32

# MSVC build
ifdef MSVC
    override CC = cl
    CXXFLAGS = /O2 /std:c++17 /D"_USRDLL" /D"WIN32" /D"_WINDOWS" \
               /D"_WIN32_WINNT=0x0600" /DWIN32_LEAN_AND_MEAN \
               /W3 /MD /LD
    LDFLAGS  = /link /DLL /OUT:$(TARGET)
    LDLIBS   = winhttp.lib comctl32.lib kernel32.lib user32.lib

    .cpp.obj:
        $(CC) /c $< $(CXXFLAGS)

    .rc.res:
        rc $<

    $(TARGET): $(SRCS:.cpp=.obj) $(RES_OBJ)
        $(CC) $(SRCS:.cpp=.obj) $(RES_OBJ) $(CXXFLAGS) $(LDFLAGS) $(LDLIBS)
else
    # MinGW build rules
    .cpp.o:
        $(CC) -c $< $(CXXFLAGS)

    .rc.o:
        $(WINDRES) $< -O coff -o $@

    $(TARGET): $(OBJS) $(RES_OBJ)
        $(CC) -o $@ $(OBJS) $(RES_OBJ) $(LDFLAGS) $(LDLIBS)
endif

.PHONY: all clean

all: $(TARGET)

clean:
ifdef MSVC
    -del /Q *.obj *.res *.dll *.lib *.exp 2>NUL
else
    rm -f $(OBJS) $(RES_OBJ) $(TARGET) $(TARGET:.dll=.a)
endif

install: $(TARGET)
ifdef MSVC
    @echo "Copy $(TARGET) to your TrafficMonitor plugins/ directory"
else
    cp $(TARGET) /path/to/TrafficMonitor/plugins/ 2>/dev/null || \
    echo "Copy $(TARGET) to your TrafficMonitor plugins/ directory"
endif
