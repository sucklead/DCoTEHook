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

BOOL write_xml_to_file(const char* filepath, const char* xmldata, SIZE_T datasize)
{
    FILE* fp = fopen(filepath, "wb");
    if (fp != NULL)
    {
        //fputs(xmldata, fp);
        fwrite(xmldata, sizeof(char), datasize, fp);
        fclose(fp);
        return true;
    }
    return false;
}

BOOL read_xml_from_file(const char* filepath, char* xmldata, SIZE_T expectedFilesize)
{
    FILE* fp = fopen(filepath, "rb");
    if (fp != NULL)
    {
        char message[1000];
        // Get the file length
        fseek(fp, 0, SEEK_END);
        SIZE_T filelength = ftell(fp);
        //fseek(fp, 0, SEEK_SET);
        rewind(fp);

        //sprintf(message, "File is %d bytes", filelength);
        //MessageBox(NULL, message, "read_xml_from_file", MB_OK | MB_TOPMOST);

        if (filelength > expectedFilesize)
        {
            sprintf(message, "File size %d is greater than max size of %d for file %s!", filelength, expectedFilesize, filepath);
            MessageBox(NULL, message, "DCOTEHook", MB_OK | MB_TOPMOST);
            return false;
        }

        //fgets(xmldata, strlen(xmldata), fp);
        SIZE_T bytesread = fread(xmldata, sizeof(char), filelength, fp);

        //sprintf(message, "File is %d bytes, read %d bytes", filelength, bytesread);
       // MessageBox(NULL, message, "read_xml_from_file", MB_OK | MB_TOPMOST);

        //terminate at read length which may be shorted than expected
        xmldata[bytesread] = 0;
        
        fclose(fp);
        return true;
    }
    return false;
}

void ImportXML(HANDLE process, const INT_PTR baseaddress, const char* filename, const char* xmlSection, const INT_PTR startAddress, const INT_PTR endAddress)
{
    SIZE_T byteswritten = 0;
    INT_PTR writeAddress = baseaddress + startAddress;
    //SIZE_T bytesToRead = endAddress - startAddress;
    SIZE_T xmlSectionBytes = endAddress - startAddress + 1;
    char* xmlbuffer = (char*)malloc(sizeof(char) * (xmlSectionBytes + 1));

    char message[1000];
    //sprintf(message, "xmlbuffer is %d bytes", bytesToWrite + 1);
    //MessageBox(NULL, message, "ImportXML", MB_OK | MB_TOPMOST);


    if (read_xml_from_file(filename, xmlbuffer, xmlSectionBytes))
    {
        //add string terminator
        //xmlbuffer[bytesToWrite] = 0;

        //void* pReservedSpace = VirtualAllocEx(process, (LPVOID)writeAddress, bytesToWrite, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
        //WriteProcessMemory(process, pReservedSpace, dllPath, strlen(dllPath), NULL);

        DWORD oldprotect;
        VirtualProtectEx(process, (LPVOID)writeAddress, xmlSectionBytes, PAGE_EXECUTE_READWRITE, &oldprotect);

        //BOOL success = WriteProcessMemory(process, reinterpret_cast<void*>(writeAddress), xmlbuffer, bytesToWrite, &byteswritten);
        BOOL success = WriteProcessMemory(process, (LPVOID)writeAddress, xmlbuffer, xmlSectionBytes, &byteswritten);
        //BOOL success = WriteProcessMemory(process, pReservedSpace, xmlbuffer, bytesToWrite, &byteswritten);
        if (success)
        {
            ////MessageBox(NULL, message, "File name", MB_OK | MB_TOPMOST);
        }
        else
        {
            //MessageBox(NULL, "Failed to write memory!", "ImportXML", MB_OK | MB_TOPMOST);

            DWORD lasterror = GetLastError();
            char stringvalue[100];
            sprintf(stringvalue, "%d", lasterror);

            MessageBox(NULL, stringvalue, "ImportXML", MB_OK | MB_TOPMOST);
        }

        VirtualProtectEx(process, (LPVOID)writeAddress, xmlSectionBytes, oldprotect, &oldprotect);
        //VirtualFreeEx(process, pReservedSpace, bytesToWrite, MEM_COMMIT);
    }
    else
    {
        MessageBox(NULL, "Failed to read xml from file!", "ImportXML", MB_OK | MB_TOPMOST);
    }
    free(xmlbuffer);
}

void ExportXML(HANDLE process, const INT_PTR baseaddress, const char* filename, const char* xmlSection, const INT_PTR startAddress, const INT_PTR endAddress)
{
    SIZE_T bytesread = 0;
    INT_PTR readAddress = baseaddress + startAddress;
    //SIZE_T bytesToRead = endAddress - startAddress + 4;
    SIZE_T xmlSectionBytes = (endAddress - startAddress) + 1;
    char message[1000];

    //sprintf(message, "Setting buffer size");
    //MessageBox(NULL, message, "HOOK main", MB_OK | MB_TOPMOST);

    //char* xmlbuffer = new char[bytesToRead];

    //allocate enough for section bytes plus terminator
    char* xmlbuffer = (char*)malloc(sizeof(char) * (xmlSectionBytes + 1));
    //memset(xmlbuffer, 0, bytesToRead * sizeof(char));

    //sprintf(message, "ReadProcessMemory");
    //MessageBox(NULL, message, "HOOK main", MB_OK | MB_TOPMOST);


    //read the process memory and look for the <DCoTESettings> tag
    BOOL success = ReadProcessMemory(process, (LPVOID)(readAddress), xmlbuffer, xmlSectionBytes, &bytesread);
    //WriteProcessMemory(processInformation.hProcess, 
    if (success)
    {
        //add string terminator after section bytes
        xmlbuffer[bytesread] = 0;
        //sprintf(message, "[%s] Success asked for %d bytes and read %d bytes, string length is %d", filename, bytesToRead, bytesread, strlen(xmlbuffer));
        //MessageBox(NULL, "Success :)", "HOOK main", MB_OK | MB_TOPMOST);
        //MessageBox(NULL, message, "HOOK main", MB_OK | MB_TOPMOST);

        //sprintf(message, "Read value [%s]", xmlbuffer);
        
        //LPCTSTR basePath = "..\\Settings\\";
        //LPCTSTR extension = ".xml";

        //int total_length = strlen(basePath) + strlen(filename) + strlen(extension) + 1;
        //char* lpWriteFilename = (char*)malloc(total_length);

        //sprintf(message, "size of filename is %d", total_length);
        ////MessageBox(NULL, message, "File name", MB_OK | MB_TOPMOST);

        //sprintf(lpWriteFilename, "%s%s%s", basePath, filename, extension);

        //MessageBox(NULL, lpWriteFilename, "File name", MB_OK | MB_TOPMOST);

        write_xml_to_file(filename, xmlbuffer, xmlSectionBytes);

        //free(lpWriteFilename);

        //MessageBox(NULL, message, "HOOK main", MB_OK | MB_TOPMOST);
    }
    else
    {
        MessageBox(NULL, "Failed!", "HOOK main", MB_OK | MB_TOPMOST);

        DWORD lasterror = GetLastError();
        char stringvalue[100];
        sprintf(stringvalue, "%d", lasterror);

        MessageBox(NULL, stringvalue, "HOOK main", MB_OK | MB_TOPMOST);
    }
    free(xmlbuffer);
}

void ProcessXML(HANDLE process, const INT_PTR baseaddress, const char* filename, const char* xmlSection, const INT_PTR startAddress, const INT_PTR endAddress)
{
    if (!FileExists(filename))
    {
        ExportXML(process, baseaddress, filename, xmlSection, startAddress, endAddress);
    }
    else
    {
        ImportXML(process, baseaddress, filename, xmlSection, startAddress, endAddress);
    }
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
        //char lpBuffer[MAX_PATH];
        //GetCurrentDirectory(MAX_PATH, lpBuffer);
        SetCurrentDirectoryA("S:\\Games\\Call Of Cthulhu DCoTE\\mods\\Engine");
        if (!FileExists("CoCMainWin32.exe"))
        {
            MessageBox(NULL, "Couldn't locate CoCMainWin32.exe in mod directory!", "DCoTEHook.exe", NULL);
            return 1;
        }
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

    CreateProcessA(0, "CoCMainWin32.exe", 0, 0, 1, CREATE_SUSPENDED | CREATE_NEW_CONSOLE, 0, 0, &startupInfo, &processInformation);

    void* pReservedSpace = VirtualAllocEx(processInformation.hProcess, NULL, strlen(dllPath), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(processInformation.hProcess, pReservedSpace, dllPath, strlen(dllPath), NULL);

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

    //MessageBox(NULL, "Getting memory...", "HOOK main", MB_OK | MB_TOPMOST);

    //MessageBox(NULL, "Creating thread...", "HOOK main", NULL);
    //char* value = "               \0";
    //SIZE_T bytesread = 0;
    //LPCVOID pointer = 0;

    //INT_PTR baseAddress = 0x6425F0;

    //0x6425F0 = 6563312
    //0x400000 = 4194304
    //0x2425f0 = 2369008
    const INT_PTR baseaddress = 0x400000;
    //const INT_PTR DCoTESettings_start = 2369008;
    //const INT_PTR DCoTESettings_end = 2388295;

    
    //unsigned long baseAddress = 0x6425F0;
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\settings.xml", "DCoTESettings", 2369008, 2388295 + 2);
    //LangRemap
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\LanguageRemap.xml", "LangRemap", 2395928, 2396562);
    //WeaponAndDamageData
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Weapons.xml", "WeaponAndDamageData", 2396568, 2400578);
    //SoundData
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\SoundData.xml", "SoundData", 2400584, 2401924);
    //PoolSizes
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\PoolSizes.xml", "PoolSizes", 2401936, 2419345 + 4);
    //MythosRanking
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\MythosRanking.xml", "MythosRanking", 2419360, 2423708);
    //SoundData
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\MusicData.xml", "SoundData", 2423720, 2440279);
    //MemoryLayout
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\MemoryLayout.xml", "MemoryLayout", 2440288, 2466404 + 4);
    //Materials
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Materials.xml", "Materials", 2466416, 2483352 + 2);
    //LevelSettings
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Levels.xml", "LevelSettings", 2483360, 2517590 + 2);
    //Journal
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Journal.xml", "Journal", 2517600, 2518100);
    //Inventory
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Inventory.xml", "Inventory", 2518112, 2530047 + 2);
    //DebugSettings
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\DebugUser.xml", "DebugSettings", 2530056, 2530128 + 2);
    //DebugScripts
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\DebugScripts.xml", "DebugScripts", 2530136, 2537597);
    //DebugCulling
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\DebugCulling.xml", "DebugCulling", 2537608, 2537828 + 2);
    //Cinematics
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Cinematics.xml", "Cinematics", 2537840, 2538506 + 2);
    //Category0
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Category0.xml", "Category", 2538520, 2543333);
    //Category1
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Category1.xml", "Category", 2543344, 2562311);
    //Category2
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\Category2.xml", "Category", 2562320, 2576933);
    //BonusItems
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\BonusItem.xml", "BonusItems", 2576944, 2578221 + 2);
    //BloodData
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\BloodSettings.xml", "BloodData", 2578232, 2578340);
    //Anim_Settings
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\AnimSettings.xml", "Anim_Settings", 2578352, 2595597);
    //Tactics_Settings
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\AITacticsSettings.xml", "Tactics_Settings", 2595608, 2600485 + 2);
    //AI_Speed_Settings
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\AISpeedSettings.xml", "AI_Speed_Settings", 2600496, 2618969 + 2);
    //AI_Settings
    ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\AICharacterTypeSettings.xml", "AI_Settings", 2620320, 2661040);
    //ProcessXML(processInformation.hProcess, baseaddress, "..\\Settings\\AICharacterTypeSettings2.xml", "AI_Settings", 2620320, 2661040);
    //ExportXML(processInformation.hProcess, baseaddress, "..\\Settings\\AICharacterTypeSettings2.xml", "AI_Settings", 2620320, 2661040);
    //ExportXML(processInformation.hProcess, baseaddress, "..\\Settings\\settings2.xml", "DCoTESettings", 2369008, 2388295 + 2);

    //MessageBox(NULL, "Here!", "DCoTEHook.exe", NULL);

    //TerminateProcess(processInformation.hProcess, 0);
    ResumeThread(processInformation.hThread);

    return 0;
}
