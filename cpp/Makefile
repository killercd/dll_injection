CXX := x86_64-w64-mingw32-g++
CC := x86_64-w64-mingw32-gcc
CXXFLAGS := -Wall -Wextra -O2
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -static -static-libgcc -static-libstdc++

DLL_SRC := dll.c
LIB_SRC := DLLInjection.cpp
MAIN_SRC := main.cpp

DLL_OUT := dll.dll
LIB_OUT := libdllinjection.a
MAIN_OUT := main.exe

.PHONY: all clean dll lib main

all: $(DLL_OUT) $(LIB_OUT) $(MAIN_OUT)

dll: $(DLL_OUT)

lib: $(LIB_OUT)

main: $(MAIN_OUT)

$(DLL_OUT): $(DLL_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $<

$(LIB_OUT): $(LIB_SRC) DLLInjection.hpp
	$(CXX) $(CXXFLAGS) -c $(LIB_SRC) -o DLLInjection.o
	x86_64-w64-mingw32-ar rcs $@ DLLInjection.o

$(MAIN_OUT): $(MAIN_SRC) $(LIB_OUT)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(MAIN_SRC) $(LIB_OUT)

clean:
	rm -f $(DLL_OUT) $(LIB_OUT) $(MAIN_OUT) DLLInjection.o
