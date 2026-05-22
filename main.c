#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

static DWORD find_process_id(const char *name) {
    PROCESSENTRY32 pe = { .dwSize = sizeof(pe) };
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    for (BOOL ok = Process32First(snap, &pe); ok; ok = Process32Next(snap, &pe)) {
        if (lstrcmpiA(pe.szExeFile, name) == 0) {
            CloseHandle(snap);
            return pe.th32ProcessID;
        }
    }

    CloseHandle(snap);
    return 0;
}

static int inject_dll(DWORD pid, const char *dll_path) {
    int result = 1;
    SIZE_T path_len = lstrlenA(dll_path) + 1;
    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
                                 PROCESS_VM_WRITE, FALSE, pid);

    if (!process) {
        printf("OpenProcess failed: %lu\n", GetLastError());
        return 1;
    }

    LPVOID remote_path = VirtualAllocEx(process, NULL, path_len,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_path) {
        printf("VirtualAllocEx failed: %lu\n", GetLastError());
        goto cleanup_process;
    }

    if (!WriteProcessMemory(process, remote_path, dll_path, path_len, NULL)) {
        printf("WriteProcessMemory failed: %lu\n", GetLastError());
        goto cleanup_memory;
    }

    FARPROC load_library = GetProcAddress(GetModuleHandleA("kernel32.dll"),
                                          "LoadLibraryA");
    if (!load_library) {
        printf("LoadLibraryA lookup failed: %lu\n", GetLastError());
        goto cleanup_memory;
    }

    HANDLE thread = CreateRemoteThread(process, NULL, 0,
                                       (LPTHREAD_START_ROUTINE)(void *)load_library,
                                       remote_path, 0, NULL);
    if (!thread) {
        printf("CreateRemoteThread failed: %lu\n", GetLastError());
        goto cleanup_memory;
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    result = 0;

cleanup_memory:
    VirtualFreeEx(process, remote_path, 0, MEM_RELEASE);
cleanup_process:
    CloseHandle(process);
    return result;
}

int main(void) {
    char dll_path[MAX_PATH];
    DWORD pid = find_process_id("notepad.exe");

    if (!pid) {
        puts("notepad.exe not found");
        return 1;
    }

    if (!GetFullPathNameA("dll.dll", MAX_PATH, dll_path, NULL)) {
        printf("GetFullPathNameA failed: %lu\n", GetLastError());
        return 1;
    }

    printf("Injecting %s into PID %lu\n", dll_path, pid);
    return inject_dll(pid, dll_path);
}
