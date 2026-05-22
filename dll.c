#include <windows.h>

BOOL APIENTRY DllMain(HMODULE module, DWORD reason, LPVOID reserved) {
    (void)module;
    (void)reserved;

    if (reason == DLL_PROCESS_ATTACH) {
        MessageBoxA(NULL, "Hello from DLL", "DLL", MB_OK);
    }

    return TRUE;
}
