CC := x86_64-w64-mingw32-gcc
AR := x86_64-w64-mingw32-ar
CFLAGS := -Wall -Wextra -O2
LDFLAGS := -static -static-libgcc

DLL_SRC := dll.c
LIB_SRC := DLLInjection.c
MAIN_SRC := main.c

DLL_OUT := dll.dll
LIB_OUT := libdllinjection.a
MAIN_OUT := main.exe
LIB_OBJ := DLLInjection.o

.PHONY: all clean dll lib main

all: $(DLL_OUT) $(LIB_OUT) $(MAIN_OUT)

dll: $(DLL_OUT)

lib: $(LIB_OUT)

main: $(MAIN_OUT)

$(DLL_OUT): $(DLL_SRC)
	$(CC) $(CFLAGS) -shared -o $@ $<

$(LIB_OUT): $(LIB_SRC) DLLInjection.h
	$(CC) $(CFLAGS) -c $(LIB_SRC) -o $(LIB_OBJ)
	$(AR) rcs $@ $(LIB_OBJ)

$(MAIN_OUT): $(MAIN_SRC) $(LIB_OUT)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(MAIN_SRC) $(LIB_OUT)

clean:
	rm -f $(DLL_OUT) $(LIB_OUT) $(MAIN_OUT) $(LIB_OBJ)
