#include "process_win32.h"

CProcessWinIO::CProcessWinIO(const CProcessMemento& process)
    : IProcessIO{ process } { }

CProcessWinIO::CProcessWinIO(std::uint32_t id)
    : IProcessIO{ id } { }

bool CProcessWinIO::readToBuffer(std::uint64_t address, std::uint32_t size, void* buffer) const {
    if(!isAttached())
        return { };

    return ReadProcessMemory(handle(), reinterpret_cast<LPCVOID>(address), buffer, size, std::nullptr_t());
}

bool CProcessWinIO::writeFromBuffer(std::uint64_t address, std::uint32_t size, void* buffer) const {
    return WriteProcessMemory(handle(), reinterpret_cast<LPVOID>(address), buffer, size, std::nullptr_t());
}

MBIEx CProcessWinIO::query(std::uint64_t address) const {
    if(!isAttached())
        return { };

    MEMORY_BASIC_INFORMATION mbi{ };
    if(!VirtualQueryEx(handle(), reinterpret_cast<LPCVOID>(address), &mbi, sizeof(MEMORY_BASIC_INFORMATION))) {
        printf("Critical: VirtualQueryEx failed (%d)\n", GetLastError());
        return { };
    }
    return MBIEx{ mbi };
}

std::tuple<bool, std::uint32_t> CProcessWinIO::protect(std::uint64_t address, std::uint32_t size, std::uint32_t flags) const {
    if(!isAttached())
        return { };

    std::uint32_t oldProtect{ };
    VirtualProtectEx(handle(), reinterpret_cast<LPVOID>(address), size, flags, reinterpret_cast<PDWORD>(&oldProtect));
    return { true, oldProtect };
}
