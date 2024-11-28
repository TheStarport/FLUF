#pragma once

#include <xbyak/xbyak.h>

struct CObject;
class CrashCatcher
{
        inline static DWORD contentModule = 0;

        // Old Function Pointers
        inline static FARPROC oldTimingSeconds = nullptr;
        inline static FARPROC oldGetRootProc = nullptr;
        inline static FARPROC crashProc6F671A0Old = nullptr;
        inline static FARPROC crashProc6F8B330Old = nullptr;
        inline static FARPROC crashProc6F78DD0Old = nullptr;

        // Logs
        static void __stdcall LogContent47bc4();

        // Naked Assembly

        struct FixEngBase11A6D final : Xbyak::CodeGenerator
        {
                FixEngBase11A6D();
        };

        struct FixEngBase124BD final : Xbyak::CodeGenerator
        {
                FixEngBase124BD();
        };

        struct FixContent47bc4 final : Xbyak::CodeGenerator
        {
                FixContent47bc4();
        };

        struct FixContent6F8B330 final : Xbyak::CodeGenerator
        {
                explicit FixContent6F8B330(void* savePtr);
        };

        struct FixContent6F78DD0 final : Xbyak::CodeGenerator
        {
                explicit FixContent6F78DD0(void* savePtr);
        };

        friend FixContent6F8B330;
        friend FixContent6F78DD0;

        // Inter-function Variables
        inline static DWORD savedEcx = 0;

        // Delay init of asm functions
        inline static std::unique_ptr<FixContent47bc4> fixContent47Bc4;
        inline static std::unique_ptr<FixContent6F8B330> fixContent6F8B330;
        inline static std::unique_ptr<FixContent6F78DD0> fixContent6F78DD0;

        // Detours

        static void CrashProc6F671A0(int arg1);
        static DWORD __stdcall EngBase11A6DCatch(const BYTE* data);
        static const BYTE* __stdcall EngBase124BDCatch(const BYTE* data);
        static CObject* GetRoot(CObject* child);
        static DWORD __stdcall C4800HookNaked();
        static int C4800Hook(int* a1, int* a2, int* zone, double* a4, int a5, int a6);
        static char __stdcall FixContentF8B330Detour(int arg1);
        static void __stdcall FixContent6F78DD0Detour(int arg1, int arg2);

    public:
        // We have to explicitly patch content over and over as switching between SP and MP will unload the dll (unpatching it)

        static void PatchContent();
        static void PatchServer();
        static void UnpatchContent();
        static void UnpatchServer();

        CrashCatcher();
        ~CrashCatcher();
};
