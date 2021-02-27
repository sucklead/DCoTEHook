# DCoTEHook by sucklead

# Intro
This is the injection hook to get DCoTE to first check in a mods directory for each file accessed.  
The main use of this hook is to allow NexusMods Vortex (https://www.nexusmods.com/site/mods/1) to manage mods for the game.  
The hooking is done using Tsuda Kageyus MinHook library (https://github.com/TsudaKageyu/minhook).
It also dumps xml settings out to a Settings dir if the don't exist and on subsequent runs loads them in instead.

# Setup
To use it follow the steps below:

## Make mods dir
Make a directory name mods in your game install at the same level as Engine, Backgrounds etc.

## Make mods\Engine dir
Under the mods dir make a directory named Engine.

## No longer need to copy files from the Engine directory
In the previous release you needed to copy 3 files in from the Engine directory but you no longer need to do that.

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
 On the first run a mods\Settings directory will be created and the xml settings from within the exe will be dumped to the following files:
 - settings.xml
 - LanguageRemap.xml
 - Weapons.xml
 - SoundData.xml
 - PoolSizes.xml
 - MythosRanking.xml
 - MusicData.xml
 - MemoryLayout.xml
 - Materials.xml
 - Levels.xml
 - Journal.xml
 - Inventory.xml
 - DebugUser.xml
 - DebugScripts.xml
 - DebugCulling.xml
 - Cinematics.xml
 - Category0.xml
 - Category1.xml
 - Category2.xml
 - BonusItem.xml
 - BloodSettings.xml
 - AnimSettings.xml
 - AITacticsSettings.xml
 - AISpeedSettings.xml
 - AICharacterTypeSettings.xml

On subsequent runs the files will instead be loaded in.  
As the files are loaded into fixed memory areas in the exe you must not exceed the original length of the file.  
You can usually remove some comments etc if you need extra space after your changes.  
The hook will warn you if you have exceeded the original length and not load the xml in.

# Build

If you want to build the source for yourself you can find it at https://github.com/sucklead/DCoTEHook.  
You can use VS2019 Community Edition to run the CMake build.
