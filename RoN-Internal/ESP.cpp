#include "ESP.hpp"
#include "Renderer.hpp"
#include "GameData.hpp"
#include "Config.hpp"
#include "Logger.hpp"

// Specific Entity Headers go here
#include "SDK/CyberneticsSuspect_V2_classes.hpp"
#include "SDK/CyberneticsSwat_V2_classes.hpp"
#include "SDK/Cybernetics_Civilian_V2_classes.hpp"
#include "SDK/ReadyOrNot_classes.hpp"

namespace ESP
{
    SDK::FLinearColor GetColorFromType(EEntityType Type, EEntityStatus Status)
    {
        if (Status == EEntityStatus::Dead) return { 0.4f, 0.4f, 0.4f, 1.0f };
        if (Status == EEntityStatus::TrapDisabled) return { 0.4f, 0.4f, 0.4f, 1.0f };
        if (Status == EEntityStatus::Arrested) {
             Config::Color C = Config::Get().ESP.ColArrested;
             return { C.R, C.G, C.B, C.A };
        }

        Config::Color C = { 1.0f, 1.0f, 1.0f, 1.0f };
        switch (Type)
        {
            case EEntityType::Suspect: C = Config::Get().ESP.ColSuspect; break;
            case EEntityType::Civilian: C = Config::Get().ESP.ColCivilian; break;
            case EEntityType::Ally: C = Config::Get().ESP.ColAlly; break;
            case EEntityType::Trap: C = Config::Get().ESP.ColTrap; break;
        }
        return { C.R, C.G, C.B, C.A };
    }

    void DrawEntity(SDK::UCanvas* Canvas, SDK::APlayerController* PC, const CachedEntity& Entity)
    {
        if (!Entity.Actor) return;
        
        SDK::FVector Origin = Entity.Actor->K2_GetActorLocation();
        
        // 2D Box Logic goes here. Not sure if there's a better way to do this
        float BoxHeightHalf = 90.0f; 
        if (Entity.Type == EEntityType::Trap) BoxHeightHalf = 25.0f;

        SDK::FVector HeadPos = Origin; HeadPos.Z += BoxHeightHalf; 
        SDK::FVector FeetPos = Origin; FeetPos.Z -= BoxHeightHalf;

        SDK::FVector2D HeadScreen, FeetScreen;
        
        if (Renderer::WorldToScreen(PC, HeadPos, HeadScreen) && 
            Renderer::WorldToScreen(PC, FeetPos, FeetScreen))
        {
            float Height = FeetScreen.Y - HeadScreen.Y;
            float Width = Height * 0.5f;
            float X = HeadScreen.X - (Width * 0.5f);
            float Y = HeadScreen.Y;

            SDK::FLinearColor Color = GetColorFromType(Entity.Type, Entity.Status);

            if (Config::Get().ESP.bBox)
            {
                Renderer::DrawBox(Canvas, { X, Y }, { Width, Height }, Config::Get().ESP.BoxThickness, Color);
            }
        }
    }

    void Draw(SDK::UCanvas* Canvas)
    {
        if (!Config::Get().ESP.bEnabled || !Canvas) return;

        SDK::UWorld* World = SDK::UWorld::GetWorld();
        if (!World || !World->OwningGameInstance || !World->OwningGameInstance->LocalPlayers[0]) return;
        SDK::APlayerController* PC = World->OwningGameInstance->LocalPlayers[0]->PlayerController;
        if (!PC) return;

        std::vector<CachedEntity> LocalList;
        
        GameData::DataMutex.lock();
        LocalList = GameData::CachedEntities;
        GameData::DataMutex.unlock();

        for (const CachedEntity& Ent : LocalList)
        {
            DrawEntity(Canvas, PC, Ent);
        }
    }
}
