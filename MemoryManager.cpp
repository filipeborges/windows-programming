#include <iostream>
#include <cstdlib>

// Windows Specific
#include <windows.h>
#include <tlhelp32.h>

class MemoryManager {
    public:
        int test();

    private:
        int handleSystemSnapshotError(HANDLE systemSnapshot);
        int saveNextProcessInfo(HANDLE systemSnapshot, LPPROCESSENTRY32 processInfo);
        void printAllProcesses(HANDLE systemSnapshot);
        void printProcessInfo(PROCESSENTRY32 processInfo);
};

int MemoryManager::test() {
    HANDLE systemSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (handleSystemSnapshotError(systemSnapshot) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    printAllProcesses(systemSnapshot);

    CloseHandle(systemSnapshot);
    return EXIT_SUCCESS;
}

void MemoryManager::printProcessInfo(PROCESSENTRY32 processInfo) {
    std::cout << "PID: " << processInfo.th32ProcessID << "\n";
    std::cout << "Exec Name: " << processInfo.szExeFile << "\n";
}

void MemoryManager::printAllProcesses(HANDLE systemSnapshot) {
    PROCESSENTRY32 processInfo;
    processInfo.dwSize = sizeof(PROCESSENTRY32);
    int returnCode;
    DWORD lastErrorCode;

    do {
        returnCode = saveNextProcessInfo(systemSnapshot, &processInfo);
        lastErrorCode = GetLastError();
        if (returnCode == EXIT_SUCCESS) {
            printProcessInfo(processInfo);
        }
    } while(returnCode != EXIT_FAILURE && lastErrorCode != ERROR_NO_MORE_FILES);
}

int MemoryManager::saveNextProcessInfo(HANDLE systemSnapshot, LPPROCESSENTRY32 processInfo) {
    processInfo->dwSize = sizeof(PROCESSENTRY32);
    BOOL isCopySuccess = Process32Next(systemSnapshot, processInfo);
    if (isCopySuccess) {
        return EXIT_SUCCESS;
    }
    CloseHandle(systemSnapshot);
    return EXIT_FAILURE;
}

int MemoryManager::handleSystemSnapshotError(HANDLE systemSnapshot) {
    if (systemSnapshot == INVALID_HANDLE_VALUE) {
        const DWORD errorCode = GetLastError();
        std::cerr << "Error creating system snapshot - Error Code: " << errorCode << '\n';
        CloseHandle(systemSnapshot);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main() {
    MemoryManager obj;
    return obj.test();
}