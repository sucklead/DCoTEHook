// HookDll.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include "include/MinHook.h"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <d3d9.h>
#include <winuser.h>
