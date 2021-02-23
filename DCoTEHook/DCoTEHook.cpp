// DCoTEHook.cpp : Defines the entry point for the application.
//

#include "DCoTEHook.h"

typedef DWORD(WINAPI* fp_NtCreateThreadEx_t)(
    PHANDLE ThreadHandle,
    ACCESS_MASK DesiredAccess,
    LPVOID ObjectAttributes,
    HANDLE ProcessHandle,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    BOOL CreateSuspended,
    DWORD dwStackSize,
    LPVOID Unknown1,
    LPVOID Unknown2,
    LPVOID Unknown3);

BOOL FileExists(LPCTSTR lpFileName)
{
    DWORD dwAttrib = GetFileAttributes(lpFileName);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

int _tmain(int argc, _TCHAR* argv[])
{
    char* dllPath = ".\\HookDll.dll";

    void* pLoadLibrary = (void*)GetProcAddress(GetModuleHandleA("kernel32"), "LoadLibraryA");

    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInformation;

    ZeroMemory(&startupInfo, sizeof(startupInfo));

    //TCHAR NPath[MAX_PATH];
    //if (GetCurrentDirectory(MAX_PATH, NPath) > 0)
    //{
    //    MessageBox(NULL, NPath, "HOOK CWD", NULL);
    //}

    if (!FileExists("CoCMainWin32.exe"))
    {
        MessageBox(NULL, "Couldn't locate CoCMainWin32.exe in mod directory!", "SucHook.exe", NULL);
        return 1;
    }

    ////MessageBox(NULL, "Finding exe...", "HOOK main", NULL);
    //if (FileExists("CoCMainWin32.exe"))
    //{
    //    //MessageBox(NULL, "Using modded CoCMainWin32.exe...", "HOOK main", NULL);
    //    //CreateProcessA("CoCMainWin32.exe", "CoCMainWin32.exe", 0, 0, 1, CREATE_NEW_CONSOLE, 0, 0, &startupInfo, &processInformation);
    //}
    ////// mod exe exists?
    ////if (FileExists("Engine\\CoCMainWin32.exe"))
    ////{
    ////    MessageBox(NULL, "Using Engine\\CoCMainWin32.exe...", "HOOK main", NULL);
    ////    CreateProcessA("CoCMainWin32.exe", "Engine\\CoCMainWin32.exe", 0, 0, 1, CREATE_NEW_CONSOLE, 0, 0, &startupInfo, &processInformation);
    ////}
    //else
    //{
    //    //MessageBox(NULL, "Using ..\\..\\Engine\\CoCMainWin32.exe...", "HOOK main", NULL);
    //    CreateProcessA("CoCMainWin32.exe", "..\\..\\Engine\\CoCMainWin32.exe", 0, 0, 1, CREATE_NEW_CONSOLE, 0, 0, &startupInfo, &processInformation);
    //}
    ////CreateProcessA(0, "notepad.exe", 0, 0, 1, CREATE_NEW_CONSOLE, 0, 0, &startupInfo, &processInformation);

    ////MessageBox(NULL, "Process created", "HOOK main", NULL);

    CreateProcessA(0, "CoCMainWin32.exe", 0, 0, 1, CREATE_NEW_CONSOLE, 0, 0, &startupInfo, &processInformation);

    void* pReservedSpace = VirtualAllocEx(processInformation.hProcess, NULL, strlen(dllPath), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(processInformation.hProcess, pReservedSpace, dllPath, strlen(dllPath), NULL);

    //MessageBox(NULL, "Creating thread...", "HOOK main", NULL);

    HANDLE hThread = NULL;
    fp_NtCreateThreadEx_t fp_NtCreateThreadEx = NULL;
    fp_NtCreateThreadEx = (fp_NtCreateThreadEx_t)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtCreateThreadEx");
    fp_NtCreateThreadEx(
        &hThread,
        0x2000000,
        NULL,
        processInformation.hProcess,
        (LPTHREAD_START_ROUTINE)pLoadLibrary,
        pReservedSpace,
        FALSE, 0, NULL, NULL, NULL);
    //MessageBox(NULL, "WaitForSingleObject...", "HOOK main", NULL);
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(processInformation.hProcess, pReservedSpace, strlen(dllPath), MEM_COMMIT);

    return 0;
}