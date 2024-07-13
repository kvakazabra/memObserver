#include "process_win32.h"

CProcessWinIO::CProcessWinIO(const CProcessMemento& process)
    : IProcessIO{ process } {
    tryAttach();
    printf("[CProcessWinIO] Attaching: %s\n", memento().name().c_str());

    m_ModuleList = std::make_unique<CModuleList>(this);
    m_ModuleList->refresh();
}

CProcessWinIO::CProcessWinIO(std::uint32_t id)
    : IProcessIO{ id } { }

CProcessWinIO::~CProcessWinIO() {
    detach();
    printf("[~CProcessWinIO] Detaching: %s\n", memento().name().c_str());
}

bool CProcessWinIO::isAttached() const {
    return Utilities::isHandleValid(handle()) && Utilities::isProcessActive(handle());
}

bool CProcessWinIO::tryAttach() {
    if(GetProcessId(GetCurrentProcess()) == memento().id())
        return false;

    m_Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, memento().id());
    if(!isAttached())
        return false;

    return true;
}

void CProcessWinIO::detach() {
    if(!isAttached())
        return;

    CloseHandle(m_Handle);
    m_Handle = INVALID_HANDLE_VALUE;
}

HANDLE CProcessWinIO::handle() const {
    return m_Handle;
}

bool CProcessWinIO::readToBuffer(std::uint64_t address, std::uint32_t size, void* buffer) const {
    if(!isAttached())
        return { };

    return ReadProcessMemory(handle(), reinterpret_cast<LPCVOID>(address), buffer, size, std::nullptr_t());
}

bool CProcessWinIO::writeFromBuffer(std::uint64_t address, std::uint32_t size, void* buffer) const {
    if(!isAttached())
        return { };

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
