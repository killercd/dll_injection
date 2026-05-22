#include "DLLInjection.hpp"

int main() {
    DLLInjection injector;
    return injector.inject("notepad.exe", "dll.dll");
}
