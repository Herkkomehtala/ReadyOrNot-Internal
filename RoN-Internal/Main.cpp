#include <Windows.h>
#include <vector>
#include <string>
#include <chrono>

#include "SDK/Engine_classes.hpp"
#include "SDK/ReadyOrNot_classes.hpp"
#include "Hooks.hpp"
#include "GameData.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include "OfficialScore.hpp"

bool IsKeyPressed(int Key) {
    return (GetAsyncKeyState(Key) & 0x8000) != 0;
}

std::string KeyToString(int vk) {
    char buffer[64];
    UINT scanCode = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
    LONG lParam = (scanCode << 16);

    switch (vk) {
        case VK_INSERT: case VK_DELETE: case VK_HOME: case VK_END:
        case VK_PRIOR: case VK_NEXT: case VK_LEFT: case VK_RIGHT:
        case VK_UP: case VK_DOWN: case VK_DIVIDE: case VK_RMENU: case VK_RCONTROL:
            lParam |= (1 << 24);
            break;
    }

    if (GetKeyNameTextA(lParam, buffer, sizeof(buffer)) > 0)
        return std::string(buffer);

    return std::to_string(vk);
}

// Helper function to isolate SEH usage
void SafeLoopBody(bool& bHookInstalled, SDK::ULevel*& LastLevel, SDK::AHUD*& IgnoreHUD, int& IgnoreHUDIndex)
{
    __try 
    {
        SDK::UEngine* Engine = SDK::UEngine::GetEngine();
        SDK::UWorld* World = SDK::UWorld::GetWorld();

        if (!Engine || (uintptr_t)Engine < 0x10000) return;
        if (!World || (uintptr_t)World < 0x10000) return;

        OfficialScore::Update(World);

        // Check if we're transitioning levels
        if (World->PersistentLevel != LastLevel)
        {
            if (LastLevel != nullptr)
            {
                // Marking the current (old) HUD as ignored so we don't re-hook it instantly :p 
                if (World->OwningGameInstance && World->OwningGameInstance->LocalPlayers[0])
                {
                     SDK::APlayerController* PC = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
                     if (PC && PC->MyHUD) {
                         IgnoreHUD = PC->MyHUD;
                         IgnoreHUDIndex = PC->MyHUD->Name.ComparisonIndex;
                     }
                }

                // After some fiddling, level change seems to destoroy the HUD. This means we need to reset the hook state.
                Hooks::Reset();
                bHookInstalled = false;

                GameData::Clear();
            }
            
            LastLevel = World->PersistentLevel;
        }

        if (World->PersistentLevel)
        {
            if (!bHookInstalled)
            {
                auto GI = World->OwningGameInstance;
                auto LP = (GI && GI->LocalPlayers.Num() > 0) ? GI->LocalPlayers[0] : nullptr;
                auto MyController = (LP) ? LP->PlayerController : nullptr;

                if (MyController)
                {
                    bool bCanHook = false;

                    // Check if pointer is usable (if pointer not valid, HUD probably is destroyed)
                    if (MyController->MyHUD == nullptr)
                    {
                        IgnoreHUD = nullptr;
                        IgnoreHUDIndex = 0;
                    }
                    else if (MyController->MyHUD != IgnoreHUD)
                    {
                        bCanHook = true;
                    }
                    else
                    {
                        // Apparently the HUD memory address can stay the same even if the previous HUD got destoyed.
                        // This means we have to check the name index
                        if (MyController->MyHUD->Name.ComparisonIndex != IgnoreHUDIndex) 
                        {
                            bCanHook = true;
                        }
                    }

                    if (bCanHook)
                    {
                        if (Hooks::Init(MyController->MyHUD))
                        {
                            bHookInstalled = true;
                        }
                        else
                        {
                            Sleep(500); // Failed to hook, might have to wait a bit for HUD to spawn
                        }
                    } 
                } 
            } 

            if (bHookInstalled) 
            {
                GameData::Update(World);
            }
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // Just a silent catch
    }
}

DWORD MainThread(HMODULE Module)
{
    Config::Load();

    if (Config::Get().Debug.bConsoleEnabled)
    {
        Logger::Init();
    }

    Logger::Log("[+] Ready or Not Internal Loaded");
    Logger::Log("[+] Config Loaded from: %s", Config::GetConfigPath().c_str());
    Logger::Log("[+] Press %s to TOGGLE ESP", KeyToString(Config::Get().Keys.ToggleESP).c_str());
    Logger::Log("[+] Press %s to TOGGLE Official Patch", KeyToString(Config::Get().Keys.ToggleOfficial).c_str());
    Logger::Log("[+] Press %s to RELOAD Config", KeyToString(Config::Get().Keys.ReloadConfig).c_str());
    Logger::Log("[+] Press %s to DETACH/UNLOAD", KeyToString(Config::Get().Keys.Detach).c_str());

    bool bRunning = true;
    bool bHookInstalled = false;
    SDK::ULevel* LastLevel = nullptr;
    SDK::AHUD* IgnoreHUD = nullptr;
    int IgnoreHUDIndex = 0;
    
    bool bEspTogglePressed = false;
    bool bOfficialTogglePressed = false;

    while (bRunning)
    {
        if (IsKeyPressed(Config::Get().Keys.Detach)) {
            Logger::Log("[!] Detach Key Pressed. Unloading...");
            bRunning = false;
            break;
        }

        if (IsKeyPressed(Config::Get().Keys.ReloadConfig)) {
            Config::Load();
            Logger::Log("[*] Config Reloaded!");
            Sleep(500); 
        }

        if (IsKeyPressed(Config::Get().Keys.ToggleESP)) {
            if (!bEspTogglePressed) {
                Config::Get().ESP.bEnabled = !Config::Get().ESP.bEnabled;
                Logger::Log("[*] ESP Toggled: %s", Config::Get().ESP.bEnabled ? "ON" : "OFF");
                bEspTogglePressed = true;
            }
        } else {
            bEspTogglePressed = false;
        }

        if (IsKeyPressed(Config::Get().Keys.ToggleOfficial)) {
            if (!bOfficialTogglePressed) {
                Config::Get().Misc.bForceOfficial = !Config::Get().Misc.bForceOfficial;
                Logger::Log("[*] Force Official Score: %s", Config::Get().Misc.bForceOfficial ? "ON" : "OFF");
                bOfficialTogglePressed = true;
            }
        } else {
            bOfficialTogglePressed = false;
        }

        if (IsKeyPressed(Config::Get().Keys.DumpEntities)) {
            Logger::Log("[*] Dumping Entities...");
            
            std::lock_guard<std::mutex> Lock(GameData::DataMutex);
            for (const auto& Ent : GameData::CachedEntities) {
                 if (!Ent.Actor) continue;
                 
                 std::string Name = Ent.Actor->GetFullName();
                 SDK::FVector Pos = Ent.Actor->K2_GetActorLocation();
                 
                 std::string TypeStr = "Unknown";
                 switch(Ent.Type) {
                     case EEntityType::Suspect: TypeStr = "Suspect"; break;
                     case EEntityType::Civilian: TypeStr = "Civilian"; break;
                     case EEntityType::Ally: TypeStr = "Ally"; break;
                     case EEntityType::Trap: TypeStr = "Trap"; break;
                 }

                 Logger::Log("Entity: %s | Type: %s | Pos: (%.0f, %.0f, %.0f)", Name.c_str(), TypeStr.c_str(), Pos.X, Pos.Y, Pos.Z);
            }
            Logger::Log("[*] Dump Complete.");
            Sleep(500);
        }

        SafeLoopBody(bHookInstalled, LastLevel, IgnoreHUD, IgnoreHUDIndex);

        Sleep(50); 
    }

    Hooks::Shutdown();
    Logger::Cleanup();
    FreeLibraryAndExitThread(Module, 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MainThread, hModule, 0, 0);
        break;
    }

    return TRUE;
}