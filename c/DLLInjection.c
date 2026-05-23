#include "DLLInjection.h"

#include <windows.h>
#include <tlhelp32.h>

#include <stdio.h>
#include <string.h>

static unsigned long find_process_id(const char *process_name) {
    PROCESSENTRY32 pe;
    HANDLE snap;
    BOOL ok;

    memset(&pe, 0, sizeof(pe));
    pe.dwSize = sizeof(pe);

    snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    for (ok = Process32First(snap, &pe); ok; ok = Process32Next(snap, &pe)) {
        if (lstrcmpiA(pe.szExeFile, process_name) == 0) {
            CloseHandle(snap);
            return pe.th32ProcessID;
        }
    }

    CloseHandle(snap);
    return 0;
}

static int inject_dll(unsigned long pid, const char *dll_path) {
    int result = 1;
    SIZE_T path_len = strlen(dll_path) + 1;
    union {
        FARPROC proc;
        LPTHREAD_START_ROUTINE thread_start;
    } load_library;
    HANDLE thread = NULL;
    LPVOID remote_path = NULL;
    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
                                 PROCESS_VM_WRITE, FALSE, pid);

    load_library.proc = NULL;

    if (!process) {
        printf("OpenProcess failed: %lu\n", GetLastError());
        return 1;
    }

    remote_path = VirtualAllocEx(process, NULL, path_len,
                                 MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remote_path) {
        printf("VirtualAllocEx failed: %lu\n", GetLastError());
        goto cleanup_process;
    }

    if (!WriteProcessMemory(process, remote_path, dll_path, path_len, NULL)) {
        printf("WriteProcessMemory failed: %lu\n", GetLastError());
        goto cleanup_memory;
    }

    load_library.proc = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!load_library.proc) {
        printf("LoadLibraryA lookup failed: %lu\n", GetLastError());
        goto cleanup_memory;
    }

    thread = CreateRemoteThread(process, NULL, 0,
                                load_library.thread_start,
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

int dll_injection_inject(const char *process_name, const char *dll_name) {
    char dll_path[MAX_PATH];
    unsigned long pid;

    if (!process_name || !dll_name) {
        printf("Invalid process or DLL name\n");
        return 1;
    }

    pid = find_process_id(process_name);
    if (!pid) {
        printf("%s not found\n", process_name);
        return 1;
    }

    if (!GetFullPathNameA(dll_name, MAX_PATH, dll_path, NULL)) {
        printf("GetFullPathNameA failed: %lu\n", GetLastError());
        return 1;
    }

    printf("Injecting %s into PID %lu\n", dll_path, pid);
    return inject_dll(pid, dll_path);
}
