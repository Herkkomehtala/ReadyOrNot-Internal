# Notes on the tool

Here's the core implementation of the internal trainer.

## Architecture

*   **`Main.cpp`**: It's the entry point. Handles the main logic thread, input handling, and the hooking loop.
*   **`Hooks.cpp`**: `AHUD::ReceiveDrawHUD` VTable hook implementation.
*   **`GameData.cpp`**: Thread-safe(r) entity caching needed to separate game memory reads from rendering.
*   **`OfficialScore.cpp`**: Memory patching logic to bypass game mod detection.

## Implementation Details

### 1. VTable and hooking

**Why:** VTables in C++ are static per class. Hooking `ReceiveDrawHUD` on one `AHUD` instance modifies the VTable for *all* instances.  
**How:**
*   We hook the current `AHUD` instance found in `UWorld`.
*   **Important note:** On level transition, the old `AHUD` is apparently destroyed. We must restore the original VTable pointer (`Hooks::Reset`) before this happens. If we don't, the *new* HUD instance created by the engine will try to call our hook function using the modified VTable. If the hook logic isn't ready or expects the old instance, this leads to undefined behavior or crash.

### 2. SEH & C++ Unwinding

**Why:**  I decided to use Structured Exception Handling to catch access violations without crashing the process. This helps us detect level transitions etc so we can restore hooks and the VTable  
**How:**
*   SEH doesn't like C++ object unwinding.
*   So we isolate all memory-intensive logic into `SafeLoopBody` functions.

### 3. Official Score Patching

**Why:** The game flags sessions as "Modded" with a few different checks, which leads to "Unofficial scoring".  
**How:** We patch four specific boolean flags:
1.  `AScoringManager::bIsOfficialScoring`
2.  `AReadyOrNotPlayerController::bClientModded`
3.  `UCommanderProfile::bIsModded`
4.  `UReadyOrNotGameInstance::ModdedLevelDataAssets`

### 4. Entity Rendering

**Why:** `UCanvas` functions are reliable but require the render thread context.  
**How:** Here we can use the `ReceiveDrawHUD` hook, which runs on the game's render thread, ensuring thread safety when calling `Canvas->K2_DrawLine` or `Canvas->K2_DrawText`.

## SDK
The `SDK/` folder contains a shortened version of the [Dumper-7](https://github.com/Encryqed/Dumper-7) output. Only headers required for compilation are included in the repository. If the game updates and the trainer doesn't work, it's probably due to new offsets. Use Dumper-7 to get new offsets.
