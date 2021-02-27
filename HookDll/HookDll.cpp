// HookDll.cpp : Defines the entry point for the application.
//

#include "HookDll.h"


typedef BOOL(WINAPI* READFILE)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef HANDLE(WINAPI* CREATEFILE)(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI* FINDFIRSTFILE)(LPCTSTR, LPWIN32_FIND_DATAA);

BOOL FileExists(LPCTSTR lpFileName)
{
    DWORD dwAttrib = GetFileAttributes(lpFileName);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL DirExists(LPCTSTR lpFileName)
{
    DWORD dwAttrib = GetFileAttributes(lpFileName);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

BOOL StartsWith(const char* a, const char* b)
{
    if (strncmp(a, b, strlen(b)) == 0) return 1;
    return 0;
}


READFILE fpReadFile = NULL;
BOOL WINAPI DetourReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
    size_t size = sizeof(FILE_NAME_INFO) + sizeof(WCHAR) * MAX_PATH;

    FILE_NAME_INFO* info = reinterpret_cast<FILE_NAME_INFO*>(malloc(size));

    memset(info, 0, size);

    info->FileNameLength = MAX_PATH;

    GetFileInformationByHandleEx(hFile, FileNameInfo, info, (DWORD)size);

    //MessageBoxW(NULL, info->FileName, L"HOOK ReadFile", NULL);
    free(info);

    return fpReadFile(hFile, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpOverlapped);
}


CREATEFILE fpCreateFile = NULL;
HANDLE WINAPI DetourCreateFile(LPCTSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
    // only handle relative paths
    if (!StartsWith(lpFileName, ".."))
    {
        return fpCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }

    LPCTSTR basePath = "..\\mods\\Engine\\";
    int total_length = strlen(basePath) + strlen(lpFileName) + 1;
    char* lpModFileName = (char*)malloc(total_length);

    strcpy(lpModFileName, basePath);
    strcat(lpModFileName, lpFileName);

    HANDLE fileHandle = NULL;

    //MessageBox(NULL, lpFileName, "lpFileName", MB_OK | MB_TOPMOST);
    //MessageBox(NULL, lpModFileName, "lpModFileName", MB_OK | MB_TOPMOST);

    // does file exist in mod directory
    if (FileExists(lpModFileName))
    {
        fileHandle = fpCreateFile(lpModFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    else
    {
        fileHandle = fpCreateFile(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
    }
    free(lpModFileName);

    return fileHandle;
}

FINDFIRSTFILE fpFindFirstFile = NULL;
//HANDLE WINAPI DetourFindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
// {
//     HANDLE fileHandle = fpFindFirstFile(lpFileName, lpFindFileData);

//     // if no files try with a .. path
//     if (fileHandle == INVALID_HANDLE_VALUE
//         && StartsWith(lpFileName, ".."))
//     {
//         LPCTSTR basePath = "..\\";
//         int total_length = strlen(basePath) + strlen(lpFileName) + 1;
//         char* lpInstallFileName = (char*)malloc(total_length);

//         strcpy(lpInstallFileName, basePath);
//         strcat(lpInstallFileName, lpFileName);
//         fileHandle = fpFindFirstFile(lpInstallFileName, lpFindFileData);

//         free(lpInstallFileName);
//     }

//     return fileHandle;
// }
HANDLE WINAPI DetourFindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
    HANDLE fileHandle = NULL;

    // we should always patch the find first file calls
    // we can redirect to mods when the files are actually opened
    // BUT only if it's a relative path request otherwise we need to just let it through
    // otherwise save file scans are broken
    if (StartsWith(lpFileName, ".."))
    {
        LPCTSTR basePath = "..\\";
        int total_length = strlen(basePath) + strlen(lpFileName) + 1;
        char* lpInstallFileName = (char*)malloc(total_length);

        strcpy(lpInstallFileName, basePath);
        strcat(lpInstallFileName, lpFileName);
        fileHandle = fpFindFirstFile(lpInstallFileName, lpFindFileData);

        free(lpInstallFileName);
    }
    else
    {
        fileHandle = fpFindFirstFile(lpFileName, lpFindFileData);
    }

    return fileHandle;
}
extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            MH_Initialize();
            //MH_CreateHook(&ReadFile, &DetourReadFile, reinterpret_cast<void**>((LPVOID)&fpReadFile));
            MH_CreateHook(&CreateFile, &DetourCreateFile, reinterpret_cast<void**>((LPVOID)&fpCreateFile));
            //MH_CreateHook(&FindFirstFile, &DetourFindFirstFile, reinterpret_cast<void**>((LPVOID)&fpFindFirstFile));

            MH_EnableHook(MH_ALL_HOOKS);
        }
        break;

        case DLL_PROCESS_DETACH:
            MH_DisableHook(MH_ALL_HOOKS);
            MH_Uninitialize();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

    }
    return true;
}

