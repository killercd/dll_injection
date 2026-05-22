#ifndef DLL_INJECTION_HPP
#define DLL_INJECTION_HPP

#include <string>

class DLLInjection {
public:
    int inject(const std::string &processName, const std::string &dllName);

private:
    static unsigned long findProcessId(const std::string &processName);
    static int injectDll(unsigned long pid, const std::string &dllPath);
};

#endif
