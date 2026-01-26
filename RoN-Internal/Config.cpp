#include "Config.hpp"
#include <filesystem>
#include <shlwapi.h>
#include <cstdio>

#pragma comment(lib, "Shlwapi.lib")

namespace Config
{
    Settings InternalSettings;

    Settings& Get()
    {
        return InternalSettings;
    }

    std::string GetConfigPath()
    {
        char Path[MAX_PATH];
        HMODULE hModule = nullptr;
        GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)&Get, &hModule);
        GetModuleFileNameA(hModule, Path, MAX_PATH);

        std::filesystem::path DllPath(Path);
        return DllPath.replace_extension(".ini").string();
    }

    // Helpers
    void WriteColor(const char* Section, const char* Key, Color& C, const std::string& Path) {
        char Buf[64];
        sprintf_s(Buf, "%.2f,%.2f,%.2f,%.2f", C.R, C.G, C.B, C.A);
        WritePrivateProfileStringA(Section, Key, Buf, Path.c_str());
    }

    Color ReadColor(const char* Section, const char* Key, Color Default, const std::string& Path) {
        char Buf[64];
        char DefBuf[64];
        sprintf_s(DefBuf, "%.2f,%.2f,%.2f,%.2f", Default.R, Default.G, Default.B, Default.A);
        
        GetPrivateProfileStringA(Section, Key, DefBuf, Buf, 64, Path.c_str());
        
        Color C = Default;
        sscanf_s(Buf, "%f,%f,%f,%f", &C.R, &C.G, &C.B, &C.A);
        return C;
    }

    void Load()
    {
        std::string Path = GetConfigPath();

        // ESP
        InternalSettings.ESP.bEnabled = GetPrivateProfileIntA("ESP", "Enabled", 1, Path.c_str());
        InternalSettings.ESP.bBox = GetPrivateProfileIntA("ESP", "Box", 1, Path.c_str());
        InternalSettings.ESP.bNameTags = GetPrivateProfileIntA("ESP", "NameTags", 1, Path.c_str());
        InternalSettings.ESP.bTraps = GetPrivateProfileIntA("ESP", "Traps", 1, Path.c_str());
        
        char Buffer[32];
        GetPrivateProfileStringA("ESP", "BoxThickness", "2.0", Buffer, 32, Path.c_str());
        InternalSettings.ESP.BoxThickness = std::stof(Buffer);

        // Colors
        InternalSettings.ESP.ColSuspect = ReadColor("ESP", "ColorSuspect", { 1.0f, 0.0f, 0.0f, 1.0f }, Path);
        InternalSettings.ESP.ColCivilian = ReadColor("ESP", "ColorCivilian", { 0.0f, 1.0f, 0.0f, 1.0f }, Path);
        InternalSettings.ESP.ColAlly = ReadColor("ESP", "ColorAlly", { 0.0f, 0.5f, 1.0f, 1.0f }, Path);
        InternalSettings.ESP.ColArrested = ReadColor("ESP", "ColorArrested", { 1.0f, 0.5f, 0.0f, 1.0f }, Path);
        InternalSettings.ESP.ColTrap = ReadColor("ESP", "ColorTrap", { 1.0f, 0.0f, 1.0f, 1.0f }, Path); // Default Purple

        // Debug
        InternalSettings.Debug.bConsoleEnabled = GetPrivateProfileIntA("Debug", "ConsoleEnabled", 1, Path.c_str());
        InternalSettings.Debug.bVerboseLog = GetPrivateProfileIntA("Debug", "VerboseLog", 0, Path.c_str());

        // Keys
        InternalSettings.Keys.ToggleESP = GetPrivateProfileIntA("Keys", "ToggleESP", VK_F1, Path.c_str());
        InternalSettings.Keys.ReloadConfig = GetPrivateProfileIntA("Keys", "ReloadConfig", VK_F5, Path.c_str());
        InternalSettings.Keys.ToggleOfficial = GetPrivateProfileIntA("Keys", "ToggleOfficial", VK_F6, Path.c_str());
        InternalSettings.Keys.Detach = GetPrivateProfileIntA("Keys", "Detach", VK_F11, Path.c_str());
        InternalSettings.Keys.DumpEntities = GetPrivateProfileIntA("Keys", "DumpEntities", VK_F9, Path.c_str());

        // Misc
        InternalSettings.Misc.bForceOfficial = GetPrivateProfileIntA("Misc", "ForceOfficial", 1, Path.c_str());
    }

    void Save()
    {
        std::string Path = GetConfigPath();

        // ESP
        WritePrivateProfileStringA("ESP", "Enabled", InternalSettings.ESP.bEnabled ? "1" : "0", Path.c_str());
        WritePrivateProfileStringA("ESP", "Box", InternalSettings.ESP.bBox ? "1" : "0", Path.c_str());
        WritePrivateProfileStringA("ESP", "NameTags", InternalSettings.ESP.bNameTags ? "1" : "0", Path.c_str());
        WritePrivateProfileStringA("ESP", "Traps", InternalSettings.ESP.bTraps ? "1" : "0", Path.c_str());
        WritePrivateProfileStringA("ESP", "BoxThickness", std::to_string(InternalSettings.ESP.BoxThickness).c_str(), Path.c_str());

        // Colors
        WriteColor("ESP", "ColorSuspect", InternalSettings.ESP.ColSuspect, Path);
        WriteColor("ESP", "ColorCivilian", InternalSettings.ESP.ColCivilian, Path);
        WriteColor("ESP", "ColorAlly", InternalSettings.ESP.ColAlly, Path);
        WriteColor("ESP", "ColorArrested", InternalSettings.ESP.ColArrested, Path);
        WriteColor("ESP", "ColorTrap", InternalSettings.ESP.ColTrap, Path);

        // Debug
        WritePrivateProfileStringA("Debug", "ConsoleEnabled", InternalSettings.Debug.bConsoleEnabled ? "1" : "0", Path.c_str());
        WritePrivateProfileStringA("Debug", "VerboseLog", InternalSettings.Debug.bVerboseLog ? "1" : "0", Path.c_str());

        // Keys
        WritePrivateProfileStringA("Keys", "ToggleESP", std::to_string(InternalSettings.Keys.ToggleESP).c_str(), Path.c_str());
        WritePrivateProfileStringA("Keys", "ReloadConfig", std::to_string(InternalSettings.Keys.ReloadConfig).c_str(), Path.c_str());
        WritePrivateProfileStringA("Keys", "ToggleOfficial", std::to_string(InternalSettings.Keys.ToggleOfficial).c_str(), Path.c_str());
        WritePrivateProfileStringA("Keys", "Detach", std::to_string(InternalSettings.Keys.Detach).c_str(), Path.c_str());
        WritePrivateProfileStringA("Keys", "DumpEntities", std::to_string(InternalSettings.Keys.DumpEntities).c_str(), Path.c_str());

        // Misc
        WritePrivateProfileStringA("Misc", "ForceOfficial", InternalSettings.Misc.bForceOfficial ? "1" : "0", Path.c_str());
    }
}
