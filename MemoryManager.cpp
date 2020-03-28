#include <iostream>
#include <cstdlib>
#include <string>

// Windows Specific
#include <windows.h>
#include <tlhelp32.h>

// TODO: Move to header file
struct MY_PROCESS {
    HANDLE processHandle;
    DWORD pid;
};
typedef MY_PROCESS* PMY_PROCESS;

class MemoryManager {
    public:
        PMY_PROCESS attachToFirstProcessEncountered(std::string desiredProcessName);

    private:
        HANDLE getSystemSnapshot();
        int saveNextProcessInfo(HANDLE systemSnapshot, LPPROCESSENTRY32 processInfo);
        void printAllProcesses(HANDLE systemSnapshot);
        void printProcessInfo(PROCESSENTRY32 processInfo);
        HANDLE openProcess(DWORD pid);
};

PMY_PROCESS MemoryManager::attachToFirstProcessEncountered(std::string desiredProcessName) {
    HANDLE systemSnapshot = getSystemSnapshot();
    if (systemSnapshot == NULL) {
        return NULL;
    }

    PROCESSENTRY32 processInfo;

    while (saveNextProcessInfo(systemSnapshot, &processInfo) == EXIT_SUCCESS) {
        if (desiredProcessName.compare(processInfo.szExeFile) == 0) {
            HANDLE processHandle = openProcess(processInfo.th32ProcessID);
            CloseHandle(systemSnapshot);
            if (processHandle == NULL) {
                return NULL;
            }
            PMY_PROCESS processData = (PMY_PROCESS)malloc(sizeof(MY_PROCESS));
            processData->pid = processInfo.th32ProcessID;
            processData->processHandle = processHandle;
            return processData;
        }
    }

    CloseHandle(systemSnapshot);
    return NULL;
}

HANDLE MemoryManager::openProcess(DWORD pid) {
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (processHandle == NULL) {
        const DWORD errorCode = GetLastError();
        std::cerr << "Error opening process - Error Code: " << errorCode << '\n';
        return NULL;
    }
    return processHandle;
}

HANDLE MemoryManager::getSystemSnapshot() {
    HANDLE systemSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (systemSnapshot == INVALID_HANDLE_VALUE) {
        const DWORD errorCode = GetLastError();
        std::cerr << "Error creating system snapshot - Error Code: " << errorCode << '\n';
        return NULL;
    }
    return systemSnapshot;
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

int main() {
    MemoryManager obj;
    PMY_PROCESS process = obj.attachToFirstProcessEncountered("Code.exe");
    if (process != NULL) {
        CloseHandle(process->processHandle);
        free(process);
    }
    return 0;
}