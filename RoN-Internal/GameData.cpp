#include "GameData.hpp"
#include "Config.hpp"
#include "SDK/ReadyOrNot_classes.hpp"
#include "SDK/CyberneticsSuspect_V2_classes.hpp"
#include "SDK/CyberneticsSwat_V2_classes.hpp"
#include "SDK/Cybernetics_Civilian_V2_classes.hpp"

std::vector<CachedEntity> GameData::CachedEntities;
std::mutex GameData::DataMutex;

void GameData::Update(SDK::UWorld* World)
{
    if (!World || !World->PersistentLevel) return;

    std::vector<CachedEntity> LocalBuffer;
    
    SDK::APawn* MyPawn = nullptr;
    if (World->OwningGameInstance && World->OwningGameInstance->LocalPlayers.Num() > 0 && World->OwningGameInstance->LocalPlayers[0])
    {
        if (auto PC = World->OwningGameInstance->LocalPlayers[0]->PlayerController)
        {
            MyPawn = PC->Pawn;
        }
    }

    // Iterate ALL levels (PersistentLevel + streaming sub-levels)
    // Evidence, weapons, etc. are often in sub-levels, not PersistentLevel
    SDK::TArray<SDK::ULevel*>& Levels = World->Levels;

    for (int lvl = 0; lvl < Levels.Num(); lvl++)
    {
        SDK::ULevel* Level = Levels[lvl];
        if (!Level) continue;

        SDK::TArray<SDK::AActor*>& Actors = Level->Actors;

        for (int i = 0; i < Actors.Num(); i++)
        {
            SDK::AActor* Actor = Actors[i];
            if (!Actor || Actor == MyPawn) continue;

        CachedEntity NewEntity;
        NewEntity.Actor = Actor;
        NewEntity.Type = EEntityType::Unknown;
        NewEntity.Status = EEntityStatus::Active;

        if (Config::Get().ESP.bTraps && Actor->IsA(SDK::ATrapActor::StaticClass()))
        {
            NewEntity.Type = EEntityType::Trap;
            SDK::ATrapActor* Trap = static_cast<SDK::ATrapActor*>(Actor);
            
            if (Trap->TrapStatus == SDK::ETrapState::TS_Live)
                NewEntity.Status = EEntityStatus::TrapLive;
            else
                NewEntity.Status = EEntityStatus::TrapDisabled;

            LocalBuffer.push_back(NewEntity);
            continue;
        }

        // AEvidenceActor (It is not the suspect's dropped weapon.)
        if (Config::Get().ESP.bEvidence && Actor->IsA(SDK::AEvidenceActor::StaticClass()))
        {
            NewEntity.Type = EEntityType::Evidence;
            NewEntity.Status = EEntityStatus::Active;
            LocalBuffer.push_back(NewEntity);
            continue;
        }

        if (Actor->IsA(SDK::AReadyOrNotCharacter::StaticClass()))
        {
            SDK::AReadyOrNotCharacter* Character = static_cast<SDK::AReadyOrNotCharacter*>(Actor);

            SDK::FVector Loc = Actor->K2_GetActorLocation();
            // Filter camera entities specifically near 0,0,-1000, allowing valid underground entities
            if (Loc.Z > -1050.0f && Loc.Z < -950.0f && 
                Loc.X > -50.0f && Loc.X < 50.0f && 
                Loc.Y > -50.0f && Loc.Y < 50.0f) 
            {
                continue; 
            }

            // Determine Status
            if (Character->IsDeadOrUnconscious()) {
                NewEntity.Status = EEntityStatus::Dead;
            }
            else if (Character->IsArrested()) {
                NewEntity.Status = EEntityStatus::Arrested;
            }

            // Determine Type
            if (Actor->IsA(SDK::ACyberneticsSuspect_V2_C::StaticClass())) {
                NewEntity.Type = EEntityType::Suspect;
            }
            else if (Actor->IsA(SDK::ACybernetics_Civilian_V2_C::StaticClass())) {
                NewEntity.Type = EEntityType::Civilian;
            }
            else if (Actor->IsA(SDK::ACyberneticsSwat_V2_C::StaticClass())) {
                NewEntity.Type = EEntityType::Ally;
            }

            if (NewEntity.Type != EEntityType::Unknown)
            {
                LocalBuffer.push_back(NewEntity);
            }
        }
        } // end actor loop
    } // end level loop

    // GameState-based detection: Dropped weapons & Reportable actors
    if (World->GameState)
    {
        auto* GS = static_cast<SDK::AReadyOrNotGameState*>(World->GameState);

        // Dropped suspect weapons: AllItems where bInInventory == false
        if (Config::Get().ESP.bDroppedWeapon)
        {
            for (int i = 0; i < GS->AllItems.Num(); i++)
            {
                auto* Item = GS->AllItems[i];
                if (!Item || Item->bInInventory) continue;

                // Only show weapons dropped by suspects
                auto* Owner = Item->PreviousOwner;
                if (!Owner || !Owner->IsA(SDK::ACyberneticsSuspect_V2_C::StaticClass())) continue;

                CachedEntity NewEntity;
                NewEntity.Actor = Item;
                NewEntity.Type = EEntityType::DroppedWeapon;
                NewEntity.Status = EEntityStatus::Active;
                LocalBuffer.push_back(NewEntity);
            }
        }

        // Reportable actors (mission-critical evidence NPCs/objects)
        // Skip already reported ones (bHasBeenReported at 0x02C1)
        if (Config::Get().ESP.bReportable)
        {
            for (int i = 0; i < GS->AllReportableActors.Num(); i++)
            {
                auto* RA = GS->AllReportableActors[i];
                if (!RA) continue;
                if (!RA->bReportableEnabled) continue;
                if (RA->bHasBeenReported) continue;

                CachedEntity NewEntity;
                NewEntity.Actor = RA;
                NewEntity.Type = EEntityType::Reportable;
                NewEntity.Status = EEntityStatus::Active;
                LocalBuffer.push_back(NewEntity);
            }
        }
    }

    std::lock_guard<std::mutex> Lock(DataMutex);
    CachedEntities = std::move(LocalBuffer);
}

void GameData::Clear()
{
    std::lock_guard<std::mutex> Lock(DataMutex);
    CachedEntities.clear();
}