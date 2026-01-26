#pragma once
#include "SDK/Engine_classes.hpp"

namespace Hooks
{
    bool Init(SDK::AHUD* TargetHUD);
    void Shutdown();
    void Reset();
}
