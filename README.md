# RoN-Internal

A simple internal tool for *Ready or Not*. This project is developed for **educational purposes only**, focusing on game engine reversal, SDK integration, and internal rendering techniques.  

[RoN-Internal ingame](assets/Internal-console.png)  

## Why?

I've been interested in how "game cheats" work so I decided to make an internal tool to find out. Ready or Not seemed like a good option as it's a game I like and I didn't see anyone making one for the 1.0 update so here's one.

## Technical Overview
- **Architecture:** Internal DLL (x64) using VTable hooking.
- **Hooking:** Intercepts `AHUD::ReceiveDrawHUD` that allows rendering on the game's `UCanvas`.
- **SDK:** SDK generated with [Dumper-7](https://github.com/Encryqed/Dumper-7)

## Features
- **ESP:** Simple 2D Box rendering with world-to-screen projection. Hooks the in-game UI so no external UI libraries like imgui needed
- **Official Score Patch:** Memory patches `AScoringManager`, `AReadyOrNotPlayerController`, and `UCommanderProfile` to bypass "Modded" session flags. Level end screen might show modded playthrough but it counts as official.
- **Configuration:** INI-based settings.

## Project Structure
- `RoN-Internal/`: Primary source code and project configuration.
- `RoN-Internal/SDK/`: Minimal SDK headers and function implementations.

## How to Use
1. **Build:** Open `RoN-Internal.sln` in Visual Studio 2022 and build the solution in **Release x64** mode.
2. **Injection:** The project creates a DLL file and doesn't include an injector. You must use your own injector. The tool should open a console when it's loaded.
3. **Configuration:** On first run, a configuration file (`RoN-Internal.ini`) will be generated in the same directory as the DLL. You can modify the file to adjust ESP colors and keybindings.

### Example Configuration (`RoN-Internal.ini`)
```ini
[ESP]
Enabled=1
Box=1
NameTags=1
Traps=1
Evidence=1
DroppedWeapon=1
Reportable=1
BoxThickness=2.0
# RGBA Floats (0.0 to 1.0)
ColorSuspect=1.00,0.00,0.00,1.00
ColorCivilian=0.00,1.00,0.00,1.00
ColorAlly=0.00,0.50,1.00,1.00
ColorArrested=1.00,0.50,0.00,1.00
ColorTrap=1.00,0.00,1.00,1.00
ColorEvidence=0.00,1.00,1.00,1.00
ColorDroppedWeapon=1.00,1.00,0.00,1.00
ColorReportable=1.00,0.50,1.00,1.00

[Keys]
# Keybindings
ToggleESP=112      ; F1
ReloadConfig=116   ; F5
ToggleOfficial=117 ; F6
DumpEntities=121   ; F10
Detach=122         ; F11

[Misc]
ForceOfficial=1

[Debug]
ConsoleEnabled=1
VerboseLog=0
```

## Credits
- [Dumper-7](https://github.com/Encryqed/Dumper-7)
- VOID Interactive :D