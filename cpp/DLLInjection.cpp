#include "DLLInjection.hpp"

#include <windows.h>
#include <tlhelp32.h>

#include <cstdio>

unsigned long DLLInjection::findProcessId(const std::string &processName) {
    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(pe);

    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snap == INVALID_HANDLE_VALUE) {
        return 0;
    }

    for (BOOL ok = Process32First(snap, &pe); ok; ok = Process32Next(snap, &pe)) {
        if (lstrcmpiA(pe.szExeFile, processName.c_str()) == 0) {
            CloseHandle(snap);
            return pe.th32ProcessID;
        }
    }

    CloseHandle(snap);
    return 0;
}

int DLLInjection::injectDll(unsigned long pid, const std::string &dllPath) {
    int result = 1;
    SIZE_T pathLen = dllPath.size() + 1;
    FARPROC loadLibrary = nullptr;
    HANDLE thread = nullptr;
    HANDLE process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
                                 PROCESS_VM_WRITE, FALSE, pid);

    if (!process) {
        std::printf("OpenProcess failed: %lu\n", GetLastError());
        return 1;
    }

    LPVOID remotePath = VirtualAllocEx(process, nullptr, pathLen,
                                       MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!remotePath) {
        std::printf("VirtualAllocEx failed: %lu\n", GetLastError());
        goto cleanupProcess;
    }

    if (!WriteProcessMemory(process, remotePath, dllPath.c_str(), pathLen, nullptr)) {
        std::printf("WriteProcessMemory failed: %lu\n", GetLastError());
        goto cleanupMemory;
    }

    loadLibrary = GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
    if (!loadLibrary) {
        std::printf("LoadLibraryA lookup failed: %lu\n", GetLastError());
        goto cleanupMemory;
    }

    thread = CreateRemoteThread(
        process, nullptr, 0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(reinterpret_cast<void *>(loadLibrary)),
        remotePath, 0, nullptr);
    if (!thread) {
        std::printf("CreateRemoteThread failed: %lu\n", GetLastError());
        goto cleanupMemory;
    }

    WaitForSingleObject(thread, INFINITE);
    CloseHandle(thread);
    result = 0;

cleanupMemory:
    VirtualFreeEx(process, remotePath, 0, MEM_RELEASE);
cleanupProcess:
    CloseHandle(process);
    return result;
}

int DLLInjection::inject(const std::string &processName, const std::string &dllName) {
    char dllPath[MAX_PATH];
    unsigned long pid = findProcessId(processName);

    if (!pid) {
        std::printf("%s not found\n", processName.c_str());
        return 1;
    }

    if (!GetFullPathNameA(dllName.c_str(), MAX_PATH, dllPath, nullptr)) {
        std::printf("GetFullPathNameA failed: %lu\n", GetLastError());
        return 1;
    }

    std::printf("Injecting %s into PID %lu\n", dllPath, pid);
    return injectDll(pid, dllPath);
}
