#pragma once
#include <vector>
#include <mutex>
#include "SDK/Engine_classes.hpp"

enum class EEntityType {
    Unknown,
    Suspect,
    Civilian,
    Ally,
    Trap
};

enum class EEntityStatus {
    Active,     // Default state
    Arrested,   // Hands cuffed
    Dead,       // Dead or Unconscious
    TrapLive,   // Live Trap
    TrapDisabled // Disarmed/Exploded
};

struct CachedEntity {
    SDK::AActor* Actor;
    EEntityType Type;
    EEntityStatus Status;
};

class GameData {
public:
    static std::vector<CachedEntity> CachedEntities;
    static std::mutex DataMutex;

    // Updates the cache. Should be called from the Main Logic Thread.
    static void Update(SDK::UWorld* World);

    // Clears the cache safely.
    static void Clear();
};