#pragma once
#include <Windows.h>
#include <cstdio>
#include <mutex>
#include <string>

namespace Logger
{
    // Shared state for all translation units
    struct State {
        bool bAllocated = false;
        std::mutex LogMutex;
    };

    // Meyers Singleton: Guarantees one instance across the DLL
    inline State& GetState() {
        static State s;
        return s;
    }

    inline void Init()
    {
        State& s = GetState();
        std::lock_guard<std::mutex> lock(s.LogMutex);

        if (s.bAllocated) return;
        
        AllocConsole();
        FILE* Dummy;
        freopen_s(&Dummy, "CONOUT$", "w", stdout);
        freopen_s(&Dummy, "CONIN$", "r", stdin);
        
        s.bAllocated = true;
        printf("[+] Console Initialized\n");
    }

    inline void Cleanup()
    {
        State& s = GetState();
        std::lock_guard<std::mutex> lock(s.LogMutex);

        if (s.bAllocated)
        {
            FreeConsole();
            if (stdout) fclose(stdout);
            if (stdin) fclose(stdin);
            s.bAllocated = false;
        }
    }

    // Variadic Log
    template <typename... Args>
    inline void Log(const char* format, Args... args)
    {
        State& s = GetState();
        if (!s.bAllocated) return;
        
        std::lock_guard<std::mutex> lock(s.LogMutex);
        printf(format, args...);
        printf("\n");
    }

    // String overload
    inline void Log(const std::string& msg)
    {
        Log(msg.c_str());
    }
    
    // Char* overload (ambiguity resolution)
    inline void Log(const char* msg)
    {
         State& s = GetState();
         if (!s.bAllocated) return;

         std::lock_guard<std::mutex> lock(s.LogMutex);
         printf("%s\n", msg);
    }
}