#pragma once
#include <Windows.h>
// https://www.vergiliusproject.com/kernels/x64/windows-10

typedef LONG KPRIORITY;

typedef struct _PROCESS_BASIC_INFORMATION {
    NTSTATUS ExitStatus;
    PVOID PebBaseAddress;
    ULONG_PTR AffinityMask;
    KPRIORITY BasePriority;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR InheritedFromUniqueProcessId;
} PROCESS_BASIC_INFORMATION;

enum PROCESSINFOCLASS  {
    ProcessBasicInformation = 0,
};

#define STATUS_SUCCESS ((NTSTATUS)0)

struct PEB64 // (Win10)
{
    UCHAR InheritedAddressSpace;                                            //0x0
    UCHAR ReadImageFileExecOptions;                                         //0x1
    UCHAR BeingDebugged;                                                    //0x2
    union
    {
        UCHAR BitField;                                                     //0x3
        struct
        {
            UCHAR ImageUsesLargePages:1;                                    //0x3
            UCHAR IsProtectedProcess:1;                                     //0x3
            UCHAR IsImageDynamicallyRelocated:1;                            //0x3
            UCHAR SkipPatchingUser32Forwarders:1;                           //0x3
            UCHAR IsPackagedProcess:1;                                      //0x3
            UCHAR IsAppContainer:1;                                         //0x3
            UCHAR IsProtectedProcessLight:1;                                //0x3
            UCHAR IsLongPathAwareProcess:1;                                 //0x3
        };
    };
    UCHAR Padding0[4];                                                      //0x4
    ULONGLONG Mutant;                                                       //0x8
    ULONGLONG ImageBaseAddress;
    ULONGLONG Ldr;
} *PPEB64;

struct UNICODE_STRING
{
    USHORT Length;                                                          //0x0
    USHORT MaximumLength;                                                   //0x2
    WCHAR* Buffer;                                                          //0x8
} *PUNICODE_STRING;

// struct LIST_ENTRY {
//     LIST_ENTRY* Flink;
//     LIST_ENTRY* Blink;
// };



struct PEB_LDR_DATA
{
    ULONG Length;                                                           //0x0
    UCHAR Initialized;                                                      //0x4
    VOID* SsHandle;                                                         //0x8
    LIST_ENTRY InLoadOrderModuleList;                               //0x10
    LIST_ENTRY InMemoryOrderModuleList;                             //0x20
    LIST_ENTRY InInitializationOrderModuleList;                     //0x30
    VOID* EntryInProgress;                                                  //0x40
    UCHAR ShutdownInProgress;                                               //0x48
    VOID* ShutdownThreadId;                                                 //0x50
} *PPEB_LDR_DATA;

struct LDR_DATA_TABLE_ENTRY
{
    LIST_ENTRY InLoadOrderLinks;                                    //0x0
    LIST_ENTRY InMemoryOrderLinks;                                  //0x10
    LIST_ENTRY InInitializationOrderLinks;                          //0x20
    VOID* DllBase;                                                          //0x30
    VOID* EntryPoint;                                                       //0x38
    ULONG SizeOfImage;                                                      //0x40
    UNICODE_STRING FullDllName;                                     //0x48
    UNICODE_STRING BaseDllName;                                     //0x58
} *PLDR_DATA_TABLE_ENTRY;
