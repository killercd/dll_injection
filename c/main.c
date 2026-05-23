#include "DLLInjection.h"

int main(void) {
    return dll_injection_inject("notepad.exe", "dll.dll");
}
