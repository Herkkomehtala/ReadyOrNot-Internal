#pragma once
#include <string>
#include <Windows.h>

namespace Config
{
    struct Color {
        float R, G, B, A;
    };

    struct ESPSettings {
        bool bEnabled = true;
        bool bBox = true;
        bool bNameTags = true;
        bool bTraps = true;
        bool bEvidence = true;
        bool bDroppedWeapon = true;
        bool bReportable = true;
        float BoxThickness = 1.0f;

        Color ColSuspect = { 1.0f, 0.0f, 0.0f, 1.0f };
        Color ColCivilian = { 0.0f, 1.0f, 0.0f, 1.0f };
        Color ColAlly = { 0.0f, 0.5f, 1.0f, 1.0f };
        Color ColArrested = { 1.0f, 0.5f, 0.0f, 1.0f };
        Color ColTrap = { 1.0f, 0.0f, 1.0f, 1.0f };
        Color ColEvidence = { 0.0f, 1.0f, 1.0f, 1.0f };
        Color ColDroppedWeapon = { 1.0f, 1.0f, 0.0f, 1.0f };
        Color ColReportable = { 1.0f, 0.5f, 1.0f, 1.0f };
    };

    struct DebugSettings {
        bool bConsoleEnabled = true;
        bool bVerboseLog = false; 
    };

    struct Keybinds {
        int ToggleESP = VK_F1;
        int ReloadConfig = VK_F5;
        int ToggleOfficial = VK_F6;
        int Detach = VK_F11;
        int DumpEntities = VK_F9;
    };

    struct MiscSettings {
        bool bForceOfficial = true;
    };

    struct Settings {
        ESPSettings ESP;
        DebugSettings Debug;
        Keybinds Keys;
        MiscSettings Misc;
    };

    // Accessor
    Settings& Get();
    std::string GetConfigPath();

    // File Operations
    void Load();
    void Save();
}
