#include "Hooks.hpp"
#include "ESP.hpp"
#include "SDK/Engine_classes.hpp"
#include "SDK/Basic.hpp"
#include <Windows.h>
#include "Logger.hpp"

void** AHUD_VTable = nullptr;
using ProcessEvent_t = void(*)(SDK::UObject*, SDK::UFunction*, void*);
ProcessEvent_t oProcessEvent = nullptr;
bool bIsHooked = false;

// function to isolate SEH
void SafeDrawHUD(SDK::AHUD* HUD)
{
    __try
    {
        if (HUD && HUD->Canvas)
        {
            ESP::Draw(HUD->Canvas);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        // means we're skipping this frame 
    }
}

void hkProcessEvent(SDK::UObject* Caller, SDK::UFunction* Function, void* Parms)
{
    if (Caller && Function)
    {
        std::string FuncName = Function->GetName();

        // Check for DrawHUD event
        if (FuncName.find("ReceiveDrawHUD") != std::string::npos)
        {
            if (oProcessEvent)
                oProcessEvent(Caller, Function, Parms);

            if (Caller->IsA(SDK::AHUD::StaticClass()))
            {
                SafeDrawHUD(static_cast<SDK::AHUD*>(Caller));
            }
            return;
        }
    }

    if (oProcessEvent)
        oProcessEvent(Caller, Function, Parms);
}

bool Hooks::Init(SDK::AHUD* TargetHUD)
{
    if (bIsHooked) return true;
    if (!TargetHUD) return false;

    // First, get the VTable pointer
    void** VTable = *reinterpret_cast<void***>(TargetHUD);
    if (!VTable) return false;

    AHUD_VTable = VTable;
    int Index = SDK::Offsets::ProcessEventIdx;

    oProcessEvent = (ProcessEvent_t)VTable[Index];

    Logger::Log("[+] Found Active HUD: %s", TargetHUD->GetName().c_str());
    Logger::Log("[+] VTable: %p | Idx: %d", VTable, Index);

    // Then we Hook by swapping the pointer in VTable
    DWORD OldProtection;
    if (VirtualProtect(&VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &OldProtection))
    {
        VTable[Index] = (void*)hkProcessEvent;
        VirtualProtect(&VTable[Index], sizeof(void*), OldProtection, &OldProtection);
        Logger::Log("[+] Hooked ProcessEvent on Active HUD successfully!");
        bIsHooked = true;
        return true;
    }
    
    Logger::Log("[!] Failed to change VTable protection");
    return false;
}

void Hooks::Reset()
{
    // IMPORTANT: We must restore the VTable because VTables are shared across instances.
    // If it's not restored, the new HUD will use our hook, but our `oProcessEvent` will be null, causing a crash or other undefined behavior
    if (bIsHooked && AHUD_VTable && oProcessEvent)
    {
        int Index = SDK::Offsets::ProcessEventIdx;
        DWORD OldProtection;
        if (VirtualProtect(&AHUD_VTable[Index], sizeof(void*), PAGE_EXECUTE_READWRITE, &OldProtection))
        {
            AHUD_VTable[Index] = (void*)oProcessEvent;
            VirtualProtect(&AHUD_VTable[Index], sizeof(void*), OldProtection, &OldProtection);
            Logger::Log("[-] VTable Restored during Reset.");
        }
    }

    // Clear state
    bIsHooked = false;
    AHUD_VTable = nullptr;
    oProcessEvent = nullptr;
    Logger::Log("[*] Hooks Reset (Ready for new level)");
}

void Hooks::Shutdown()
{
    Hooks::Reset();
}
