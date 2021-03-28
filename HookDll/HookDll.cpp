// HookDll.cpp : Defines the entry point for the application.
//

#include "HookDll.h"

typedef BOOL(WINAPI* READFILE)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef HANDLE(WINAPI* CREATEFILE)(LPCTSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);
typedef HANDLE(WINAPI* FINDFIRSTFILE)(LPCTSTR, LPWIN32_FIND_DATAA);

typedef LONG(WINAPI* CHANGEDISPLAYSETTINGS)(LPDEVMODE, DWORD);
typedef HWND(WINAPI* CREATEWINDOWEX)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int, HWND, HMENU, HINSTANCE, LPVOID);
typedef BOOL(WINAPI* SETWINDOWPOS)(HWND, HWND, int, int, int, int, UINT);

char message[1024];

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

void ClearLogFile()
{
    fclose(fopen("DCoTEHook.log", "w"));
}

BOOL write_to_log_file(const char* logText)
{
    const char* filepath = "DCoTEHook.log";
    FILE* fp = fopen(filepath, "a");
    if (fp != NULL)
    {
        SIZE_T datasize = strlen(logText);
        fwrite(logText, sizeof(char), datasize, fp);
        fclose(fp);
        return true;
    }
    return false;
}

// By Timb3r
// Source: https://gamephreakers.com/2018/08/introduction-to-vtable-hooking/
void* HookVTableFunction(void* pVTable, void* fnHookFunc, int nOffset)
{
    intptr_t ptrVtable = *((intptr_t*)pVTable); // Pointer to our chosen vtable
    intptr_t ptrFunction = ptrVtable + sizeof(intptr_t) * nOffset; // The offset to the function (remember it's a zero indexed array with a size of four bytes)
    intptr_t ptrOriginal = *((intptr_t*)ptrFunction); // Save original address

    // Edit the memory protection so we can modify it
    MEMORY_BASIC_INFORMATION mbi;
    VirtualQuery((LPCVOID)ptrFunction, &mbi, sizeof(mbi));
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &mbi.Protect);

    // Overwrite the old function with our new one
    *((intptr_t*)ptrFunction) = (intptr_t)fnHookFunc;

    // Restore the protection
    VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

    // Return the original function address incase we want to call it
    return (void*)ptrOriginal;
}

BOOL isWindowed = false;
HWND hD3DProxyWindow = NULL;
HWND hCoCDCOTE = NULL;

SETWINDOWPOS fpSetWindowPos = NULL;
BOOL WINAPI DetourSetWindowPos(HWND hWnd, HWND hWndInsertAfter, int X, int Y, int cx, int cy, UINT uFlags)
{
    //char windowTitle[1025];

    //if (GetWindowTextA(hWnd, windowTitle, 1024))
    //{
    //    if (strcmp("D3DProxyWindow", windowTitle) == 0)
    //    {
    //        sprintf(message, "Stored D3DProxyWindow hWnd=%d\n", (int)hWnd);
    //        write_to_log_file(message);
    //        hD3DProxyWindow = hWnd;
    //    }
    //    if (strcmp("CoCDCOTE", windowTitle) == 0)
    //    {
    //        sprintf(message, "Stored CoCDCOTE hWnd=%d\n", (int)hWnd);
    //        write_to_log_file(message);
    //        hCoCDCOTE = hWnd;
    //    }


    //}

    //sprintf(message, "IN DetourSetWindowPos hWnd=%d\n", (int)hWnd);
    //write_to_log_file(message);
    //if (hWndInsertAfter == HWND_TOP)
    //{
    //    write_to_log_file("Setting flags");
    //    SetWindowLongA(hWnd, GWL_STYLE, 0x94080000);
    //    SetWindowLongA(hWnd, GWL_EXSTYLE, 0x00040000);
    //    //uFlags = SWP_SHOWWINDOW;
    //    //SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_SHOWWINDOW
    //    uFlags = SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_SHOWWINDOW;
    //}

    return fpSetWindowPos(hWnd, hWndInsertAfter, X, Y, cx, cy, uFlags);
}

CHANGEDISPLAYSETTINGS fpChangeDisplaySettings = NULL;
LONG WINAPI DetourChangeDisplaySettings(LPDEVMODE lpDevMode, DWORD dwFlags)
{
    return fpChangeDisplaySettings(lpDevMode, dwFlags);
}

CREATEWINDOWEX fpCreateWindowEx = NULL;
HWND WINAPI DetourCreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
    sprintf(message, "IN DetourCreateWindowEx lpWindowName=%s\n", lpWindowName);
    write_to_log_file(message);

    //MessageBox(NULL, "DetourCreateWindowEx Call Started", "HookDll", MB_OK | MB_TOPMOST);

    //if (!StartsWith(lpWindowName, "D3DProxyWindow"))
    {
        //dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        //dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

        //dwExStyle &= ~WS_EX_NOACTIVATE;
        //nWidth = 640;
        //nHeight = 480;

        //dwExStyle = WS_EX_APPWINDOW;
        //dwStyle = WS_CAPTION | WS_POPUPWINDOW;
    }

    HWND m_hwnd = fpCreateWindowEx(
        dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y,
        nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

    if (strcmp("D3DProxyWindow", lpWindowName) == 0)
    {
        sprintf(message, "    Stored D3DProxyWindow hWnd=%d\n", (int)m_hwnd);
        write_to_log_file(message);
        hD3DProxyWindow = m_hwnd;

        //HookDirect3D(hD3DProxyWindow);
    }
    else if (strcmp("CoCDCOTE", lpWindowName) == 0)
    {
        sprintf(message, "    Stored CoCDCOTE hWnd=%d\n", (int)m_hwnd);
        write_to_log_file(message);
        hCoCDCOTE = m_hwnd;
    }

    sprintf(message, "    hWnd=%d lpWindowName=%s\n", (int)m_hwnd, lpWindowName);
    write_to_log_file(message);

    //ShowCursor(TRUE);
    //MessageBox(NULL, "DetourCreateWindowEx Window created", "HookDll", MB_OK | MB_TOPMOST);
    return m_hwnd;
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

MH_STATUS WINAPI MH_CreateHookVirtualEx(LPVOID pInstance, UINT methodPos, LPVOID pDetour, LPVOID* ppOriginal, LPVOID* ppTarget)
{
    LPVOID* pVMT = *((LPVOID**)pInstance);
    LPVOID  pTarget = pVMT[methodPos];

    if (ppTarget != NULL)
        *ppTarget = pTarget;

    return MH_CreateHook(pTarget, pDetour, ppOriginal);
}

//-------------------------------------------------------------------------
MH_STATUS WINAPI MH_CreateHookVirtual(LPVOID pInstance, UINT methodPos, LPVOID pDetour, LPVOID* ppOriginal)
{
    return MH_CreateHookVirtualEx(pInstance, methodPos, pDetour, ppOriginal, NULL);
}

typedef HRESULT(WINAPI* IDirectInputDevice_GetDeviceState_t)(IDirectInputDevice8*, DWORD, LPVOID);
IDirectInputDevice_GetDeviceState_t fpGetDeviceState = NULL;

#define KeyDown(data, n) ((data[n] & 0x80) ? true : false)
#define KeyUp(data, n) ((data[n] & 0x80) ? false : true)

HRESULT WINAPI DetourGetDeviceState(IDirectInputDevice8* pThis, DWORD cbData, LPVOID  lpvData)
{
    //write_to_log_file("IN HookGetDeviceState\n");
    HRESULT ret = fpGetDeviceState(pThis, cbData, lpvData);
    if (ret == DI_OK)
    {
        if (KeyDown(((byte*)lpvData), DIK_F7))
        {
            if (!isWindowed)
            {
                write_to_log_file("Setting to windowed mode\n");

                isWindowed = true;
                SetWindowLongA(hD3DProxyWindow, GWL_STYLE, 0x94080000);
                SetWindowLongA(hD3DProxyWindow, GWL_EXSTYLE, 0x00040000);

                UINT uFlags = SWP_NOMOVE | SWP_NOREPOSITION | SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_SHOWWINDOW;
                //UINT uFlags = SWP_NOSENDCHANGING | SWP_NOZORDER | SWP_ASYNCWINDOWPOS | SWP_NOREPOSITION | SWP_SHOWWINDOW | SWP_NOACTIVATE;
                fpSetWindowPos(hD3DProxyWindow, HWND_TOP, 0, 0, 800, 600, uFlags);

                SetWindowLongA(hCoCDCOTE, GWL_STYLE, 0x94080000);
                SetWindowLongA(hCoCDCOTE, GWL_EXSTYLE, 0x00040000);

                fpSetWindowPos(hCoCDCOTE, HWND_TOP, 0, 0, 800, 600, uFlags);


                //if (game_window.actual.window.right - game_window.actual.window.left > 0 &&
                //    game_window.actual.window.bottom - game_window.actual.window.top > 0)
                //{
                //    SK_SetWindowPos(game_window.hWnd,
                //        SK_HWND_TOP,
                //        game_window.actual.window.left,
                //        game_window.actual.window.top,
                //        game_window.actual.window.right - game_window.actual.window.left,
                //        game_window.actual.window.bottom - game_window.actual.window.top,
                //        SWP_NOSENDCHANGING | SWP_NOZORDER |
                //        SWP_NOREPOSITION | SWP_SHOWWINDOW |
                //        (nomove ? SWP_NOMOVE : 0x00) |
                //        (nosize ? SWP_NOSIZE : 0x00) | SWP_NOACTIVATE);
                //}


            }
        }
    }
    else
    {
        write_to_log_file("    NOT DI_OK\n");
    }
    return ret;
}

IDirectInputDevice8A* lpdiKeyboard = NULL;
LPDIRECTINPUT8 pDirectInput = NULL;
BOOL HookDirectInput()
{
    write_to_log_file("IN HookDirectInput\n");

    auto result = DirectInput8Create(GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&pDirectInput, nullptr);
    if (FAILED(result))
    {
        write_to_log_file("DirectInput8Create failed\n");
        return false;
    }

    pDirectInput->CreateDevice(GUID_SysKeyboard, &lpdiKeyboard, nullptr);

    // hook vtable for direct input
    fpGetDeviceState = (IDirectInputDevice_GetDeviceState_t)HookVTableFunction(lpdiKeyboard, DetourGetDeviceState, 9);

    write_to_log_file("    DirectInput Device created!\n");

    return true;
}

BOOL UnHookDirectInput()
{
    lpdiKeyboard->Release();
    pDirectInput->Release();

    return true;
}

static HWND window;

BOOL UnHookDirect3D()
{

    return true;
}

DWORD* GetVtableAddress(void* pObject)
{
    // The first 4 bytes of the object is a pointer to the vtable:
    return (DWORD*)*((DWORD*)pObject);
}

void HookFunction(DWORD* pVtable, void* pHookProc, void* pOldProc, int iIndex)
{
    // Enable writing to the vtable at address we aquired
    DWORD lpflOldProtect;
    VirtualProtect((void*)&pVtable[iIndex], sizeof(DWORD), PAGE_READWRITE, &lpflOldProtect);

    // Store old address
    if (pOldProc) {
        *(DWORD*)pOldProc = pVtable[iIndex];
    }

    // Overwrite original address
    pVtable[iIndex] = (DWORD)pHookProc;

    // Restore protection
    VirtualProtect(pVtable, sizeof(DWORD), lpflOldProtect, &lpflOldProtect);
}

// HOOK CreateDevice
typedef HRESULT(APIENTRY* CreateDevice_t)(IDirect3D9*, UINT, D3DDEVTYPE, HWND, DWORD, D3DPRESENT_PARAMETERS*, IDirect3DDevice9**);
CreateDevice_t orig_CreateDevice;

HRESULT APIENTRY hook_CreateDevice(IDirect3D9* pInterface, UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface)
{
    write_to_log_file("IN hook_CreateDevice\n");

    pPresentationParameters->BackBufferWidth = 0;
    pPresentationParameters->BackBufferHeight = 0;
    pPresentationParameters->Flags = 0;
    pPresentationParameters->FullScreen_RefreshRateInHz = 0;
    pPresentationParameters->PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
    pPresentationParameters->Windowed = true;

    HRESULT ret = orig_CreateDevice(pInterface, Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

    // Registers MUST be preserved when doing your own stuff!!
    __asm pushad

    write_to_log_file("    D3D Device created\n");

    __asm popad

    return ret;
}

// HOOK Direct3DCreate9
// globals
// Our hook function
IDirect3D9* __stdcall hook_Direct3DCreate9(UINT sdkVers);

typedef IDirect3D9* (WINAPI* FND3DC9)(UINT);

// The original to call
typedef IDirect3D9* (__stdcall* Direct3DCreate9_t)(UINT SDKVersion);
Direct3DCreate9_t orig_Direct3DCreate9;

// Holds address that we get in our LoadLibrary hook (used for detour)
PBYTE pDirect3DCreate8;

IDirect3D9* __stdcall hook_Direct3DCreate9(UINT sdkVers)
{
    write_to_log_file("IN hook_Direct3DCreate9\n");

    write_to_log_file("    Calling real function...\n");
    IDirect3D9* pD3d9 = orig_Direct3DCreate9(sdkVers); // real one

    __asm pushad

    write_to_log_file("    Hooking vtable...\n");
    // Use a vtable hook on CreateDevice to get the device pointer later
    DWORD* pVtable = GetVtableAddress(pD3d9);
    HookFunction(pVtable, (void*)&hook_CreateDevice, (void*)&orig_CreateDevice, 16);

    //orig_CreateDevice = (CreateDevice_t)HookVTableFunction(pD3d9, hook_CreateDevice, 15);
    //HookVTableFunction(pD3d9, )

    write_to_log_file("    Hooking complete\n");

    __asm popad

    return pD3d9;
}

BOOL HookDirect3D()
{
    write_to_log_file("IN GetD3D9Device\n");

    HMODULE hDll = LoadLibrary("d3d9.dll");
    if (hDll == NULL)
    {
        write_to_log_file("    Failed to load d3d9.dll\n");
        return FALSE;
    }

    // Pointer to the original function
    FND3DC9 Direct3DCreate9_out = (FND3DC9)GetProcAddress(hDll, "Direct3DCreate9");
    if (Direct3DCreate9_out == NULL)
    {
        write_to_log_file("    GetProcAddress failed!\n");
        FreeLibrary(hDll);
        exit(1);
    }

    write_to_log_file("    Adding Hook to Direct3DCreate9...\n");

    MH_CreateHook(Direct3DCreate9_out, &hook_Direct3DCreate9, reinterpret_cast<void**>((LPVOID)&orig_Direct3DCreate9));

    write_to_log_file("    Hook added\n");

    return true;
}

extern "C" BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpvReserved)
{
    DisableThreadLibraryCalls(hinstDLL);

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
        {
            ClearLogFile();

            MH_Initialize();

            HookDirectInput();
            
            HookDirect3D();

            //auto func = (void*)get_virtual(device, 9);
            //MH_CreateHook(func, Hooks::hkGetDeviceState, (void**)&Hooks::oGetDeviceState);

            // file access
            //MH_CreateHook(&ReadFile, &DetourReadFile, reinterpret_cast<void**>((LPVOID)&fpReadFile));
            MH_CreateHook(&CreateFile, &DetourCreateFile, reinterpret_cast<void**>((LPVOID)&fpCreateFile));
            //MessageBox(NULL, "Hooking CreateWindowEx", "HookDll", MB_OK | MB_TOPMOST);

            // Hook window functions
            MH_CreateHook(&CreateWindowEx, &DetourCreateWindowEx, reinterpret_cast<void**>((LPVOID)&fpCreateWindowEx));
            //MH_CreateHook(&ChangeDisplaySettings, &DetourChangeDisplaySettings, reinterpret_cast<void**>((LPVOID)&fpChangeDisplaySettings));
            //MH_CreateHook(&SetWindowPos, &DetourSetWindowPos, reinterpret_cast<void**>((LPVOID)&fpSetWindowPos));
            //MessageBox(NULL, "CreateWindowEx Hooked", "HookDll", MB_OK | MB_TOPMOST);
            //MH_CreateHook(&FindFirstFile, &DetourFindFirstFile, reinterpret_cast<void**>((LPVOID)&fpFindFirstFile));

            // direct input
            //MH_CreateHook(&SetWindowPos, &DetourSetWindowPos, reinterpret_cast<void**>((LPVOID)&fpSetWindowPos));

            MH_EnableHook(MH_ALL_HOOKS);
        }
        break;

        case DLL_PROCESS_DETACH:
            MH_DisableHook(MH_ALL_HOOKS);

            //UnHookDirectInput();
            //UnHookDirect3D();

            MH_Uninitialize();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

    }
    return true;
}

