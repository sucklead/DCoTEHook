# DCoTEHook by sucklead

# Intro
This is the injection hook to get DCoTE to first check in a mods directory for each file accessed.  
We can also hopefully load the xml settings from a file rather than patching the exe.  
The main use of this hook is to allow NexusMods Vortex (https://www.nexusmods.com/site/mods/1) to manage mods for the game.  
The hooking is done using Tsuda Kageyus MinHook library (https://github.com/TsudaKageyu/minhook).

# Setup
To use it follow the steps below:

## Make mods dir
Make a directory name mods in your game install at the same level as Engine, Backgrounds etc.

## Make mods\Engine dir
Under the mods dir make a directory named Engine.

## Copy base Engine files into mods\Engine
From the games Engine directory copy the following files into the mods\Engine directory:
- CoCMainWin32.exe
- fmodex.dll
- gdiplus.dll

## Add hook files to mods\Engine dir
Place the following 3 files from the release into your mods\Engine directory:
- DCoTEHook.exe
- HookDll.dll
- MinHook.x86.dll
 
 # Run

 Run DCoTEHook.exe from the mods\Engine directory.  
 This will start up CoCMainWin32.exe and then use MinHook to hook its file access requests.  
 All file open requests will be checked for a matching file in the mods directory tree first before loading .  
 You should replicate the folder structure in the main directory in the mods directory for any files you want to override.

# Build

If you want to build the source for yourself you can find it at https://github.com/sucklead/DCoTEHook.  
You can use VS2019 Community Edition to run the CMake build.
