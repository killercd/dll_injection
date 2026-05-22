CC := x86_64-w64-mingw32-gcc
CFLAGS := -Wall -Wextra -O2

DLL_SRC := dll.c
MAIN_SRC := main.c

DLL_OUT := dll.dll
MAIN_OUT := main.exe

.PHONY: all clean dll main

all: $(DLL_OUT) $(MAIN_OUT)

dll: $(DLL_OUT)

main: $(MAIN_OUT)

$(DLL_OUT): $(DLL_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $<

$(MAIN_OUT): $(MAIN_SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(DLL_OUT) $(MAIN_OUT)
