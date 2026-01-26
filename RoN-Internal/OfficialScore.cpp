#include "OfficialScore.hpp"
#include "Config.hpp"
#include "Logger.hpp"
#include "SDK/ReadyOrNot_classes.hpp"
#include "SDK/UnrealContainers.hpp"

namespace OfficialScore
{
    SDK::AScoringManager* CachedManager = nullptr;
    SDK::ULevel* LastLevel = nullptr;
    
    bool LastKnownOfficialState = false;
    bool bStateInitialized = false;

    void PatchScoringManager(SDK::UWorld* World)
    {
        if (World->PersistentLevel != LastLevel)
        {
            CachedManager = nullptr;
            LastLevel = World->PersistentLevel;
            bStateInitialized = false;
        }

        if (!CachedManager)
        {
            auto& Actors = World->PersistentLevel->Actors;
            for (int i = 0; i < Actors.Num(); i++)
            {
                SDK::AActor* Actor = Actors[i];
                if (Actor && Actor->IsA(SDK::AScoringManager::StaticClass()))
                {
                    CachedManager = static_cast<SDK::AScoringManager*>(Actor);
                    Logger::Log("[+] Found ScoringManager at %p", CachedManager);
                    break;
                }
            }
        }

        if (CachedManager)
        {
            bool bCurrent = CachedManager->bIsOfficialScoring;

            if (!bStateInitialized) {
                LastKnownOfficialState = bCurrent;
                bStateInitialized = true;
                Logger::Log("[Debug] Initial Official Score State: %s", bCurrent ? "TRUE" : "FALSE");
            }
            else if (bCurrent != LastKnownOfficialState) {
                Logger::Log("[Debug] Official Score Changed: %s -> %s", 
                    LastKnownOfficialState ? "TRUE" : "FALSE", 
                    bCurrent ? "TRUE" : "FALSE");
                LastKnownOfficialState = bCurrent;
            }

            if (!bCurrent)
            {
                CachedManager->bIsOfficialScoring = true;
            }
        }
    }

    void PatchPlayerController(SDK::UWorld* World)
    {
        if (World->OwningGameInstance && World->OwningGameInstance->LocalPlayers.Num() > 0)
        {
            SDK::APlayerController* PC = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
            if (PC && PC->IsA(SDK::AReadyOrNotPlayerController::StaticClass()))
            {
                SDK::AReadyOrNotPlayerController* RonPC = static_cast<SDK::AReadyOrNotPlayerController*>(PC);
                if (RonPC->bClientModded)
                {
                    RonPC->bClientModded = false;
                    Logger::Log("[*] Un-modded Local PlayerController");
                }
            }
        }
    }

    void PatchCommanderProfile(SDK::UWorld* World)
    {
        if (World->AuthorityGameMode)
        {
            SDK::AGameModeBase* GM = World->AuthorityGameMode;
            SDK::UCommanderProfile* Profile = nullptr;

            static SDK::AGameModeBase* LastGM = nullptr;
            if (GM != LastGM)
            {
                Logger::Log("[Debug] AuthorityGameMode Class: %s", GM->Class->GetName().c_str());
                LastGM = GM;
            }

            if (GM->IsA(SDK::ACommanderGM::StaticClass()))
            {
                Profile = static_cast<SDK::ACommanderGM*>(GM)->CommanderProfile;
            }
            else if (GM->IsA(SDK::ALobbyGM::StaticClass()))
            {
                Profile = static_cast<SDK::ALobbyGM*>(GM)->CommanderProfile;
            }

            if (Profile && Profile->bIsModded)
            {
                Profile->bIsModded = false;
                Logger::Log("[*] Un-modded Commander Profile");
            }
        }
    }

    void PatchGameInstance(SDK::UWorld* World)
    {
        if (World->OwningGameInstance && World->OwningGameInstance->IsA(SDK::UReadyOrNotGameInstance::StaticClass()))
        {
            SDK::UReadyOrNotGameInstance* GI = static_cast<SDK::UReadyOrNotGameInstance*>(World->OwningGameInstance);
            if (GI->ModdedLevelDataAssets.Num() > 0)
            {
                GI->ModdedLevelDataAssets.Clear();
                Logger::Log("[*] Cleared ModdedLevelDataAssets");
            }
        }
    }

    void Update(SDK::UWorld* World)
    {
        if (!Config::Get().Misc.bForceOfficial || !World || !World->PersistentLevel) return;

        PatchScoringManager(World);
        PatchPlayerController(World);
        PatchCommanderProfile(World);
        PatchGameInstance(World);
    }
}