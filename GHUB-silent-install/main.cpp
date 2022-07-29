#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>
#include <winternl.h>
#pragma comment(lib,"ntdll.lib")
#include <filesystem>
typedef struct _SYSTEM_PROCESS_INFO
{
    ULONG                   NextEntryOffset;
    ULONG                   NumberOfThreads;
    LARGE_INTEGER           Reserved[3];
    LARGE_INTEGER           CreateTime;
    LARGE_INTEGER           UserTime;
    LARGE_INTEGER           KernelTime;
    UNICODE_STRING          ImageName;
    ULONG                   BasePriority;
    HANDLE                  ProcessId;
    HANDLE                  InheritedFromProcessId;
}SYSTEM_PROCESS_INFO, * PSYSTEM_PROCESS_INFO;


int get_process_id(LPCWSTR str) {
    UINT times = 0;
    NTSTATUS status;
    PVOID buffer;
    PSYSTEM_PROCESS_INFO spi;

    buffer = VirtualAlloc(NULL, 1024 * 1024, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!buffer)  return 0;

    spi = (PSYSTEM_PROCESS_INFO)buffer;

    if (!NT_SUCCESS(status = NtQuerySystemInformation(SystemProcessInformation, spi, 1024 * 1024, NULL))) { // Query all processes

        VirtualFree(buffer, 0, MEM_RELEASE);
        return 0;
    }

    while (spi->NextEntryOffset) { // Loop them
        if (!lstrcmpW(spi->ImageName.Buffer, str)) { // strcmp for wide chars
            times++; // Increament thread count
            if (times >= 2) { // Assert that the installation is fine and ghub decides to open menu
                int result = (int)spi->ProcessId;
                VirtualFree(buffer, 0, MEM_RELEASE);
                return result;
            }
        }
        spi = (PSYSTEM_PROCESS_INFO)((LPBYTE)spi + spi->NextEntryOffset); // Get next in list
    }

    VirtualFree(buffer, 0, MEM_RELEASE);
    return 0;
}

void execute_powershell() {
    std::ofstream file;
    file.open("test.ps1");
    std::string powershell;
    powershell += "cd C:\\\n";
    powershell += "Invoke-WebRequest -uri \"https://yourOwnFile.exe\" -OutFile ( New-Item -Path \"C:\lghub.exe\" -Force)\n";
    powershell += "Start lghub.exe --silent\n"; // silent install
    file << powershell << std::endl;
    file.close();
    system("powershell -F test.ps1");
    remove("test.ps1");
}

int main(void) {
    if (!std::filesystem::exists("C:\\Program Files\\LGHUB\\lghub.exe")) // Is it right to hard code it? Does every pc have a c driver? or do some have D drives as main?
    {
        //Here maby uninstall? We some how need to make sure we have the right version. Maby validate it with a Size check of the files?
        execute_powershell();

        std::cout << "Downloading and Installing...";
        int pid = 0;
        while (!pid) {
            pid = get_process_id(L"lghub.exe"); // Check if ghub has more then 2 or = threads. That means the menu has opend and we need to close to have background start
        }
    }

    //If threads is more then or = 2 close down lghub.exe
    system("TASKKILL /F /IM lghub.exe"); // Make sure its closed aswell
    Sleep(1000);
    // Now start with argument --background
    // The user will notice that ghub is installed but their is no installation proccess with the right version etc.
    system("cd C:\\Program Files\\LGHUB && start lghub.exe --background");
    system("pause");
	return 0;
}