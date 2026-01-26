#include "Renderer.hpp"
#include <string>
#include <vector>

namespace Renderer
{
    bool WorldToScreen(SDK::APlayerController* PC, SDK::FVector WorldPos, SDK::FVector2D& ScreenPos)
    {
        if (!PC) return false;
        return PC->ProjectWorldLocationToScreen(WorldPos, &ScreenPos, false);
    }

    void DrawBox(SDK::UCanvas* Canvas, SDK::FVector2D Pos, SDK::FVector2D Size, float Thickness, SDK::FLinearColor Color)
    {
        if (!Canvas) return;
        Canvas->K2_DrawBox(Pos, Size, Thickness, Color);
    }

    void DrawString(SDK::UCanvas* Canvas, const std::string& Text, SDK::FVector2D Pos, SDK::FLinearColor Color, bool bCentered)
    {
        if (!Canvas || Text.empty()) return;

        SDK::FString Str = SDK::FString(std::wstring(Text.begin(), Text.end()).c_str());
        
        Canvas->K2_DrawText(nullptr, Str, Pos, { 1.0f, 1.0f }, Color, 1.0f, { 0,0,0,1 }, { 1,1 }, bCentered, false, true, { 0,0,0,1 });
    }

    void DrawLine(SDK::UCanvas* Canvas, SDK::FVector2D Start, SDK::FVector2D End, float Thickness, SDK::FLinearColor Color)
    {
        if (!Canvas) return;
        Canvas->K2_DrawLine(Start, End, Thickness, Color);
    }
}