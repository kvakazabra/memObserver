#include "process.h"

#include <TlHelp32.h>

CProcessMemento::CProcessMemento(const std::uint32_t id, const std::string& name)
    : m_Id{ id }
    , m_Name{ name } { }

CProcessMemento::CProcessMemento(CProcessMemento &&mv) {
    *this = std::move(mv);
};

CProcessMemento& CProcessMemento::operator=(CProcessMemento&& mv) {
    this->m_Id = mv.m_Id;
    this->m_Name = std::move(mv.m_Name);
    this->m_Description = std::move(mv.m_Description);
    return *this;
}

int operator<=>(const CProcessMemento& a1, const CProcessMemento& a2) {
    return a1.m_Id - a2.m_Id;
}

std::string CProcessMemento::format() const {
    //char buffer[512]{ };
    //sprintf_s(buffer, "[%d] %s - %s", m_Id, m_Name.c_str(), m_Description.c_str());

    char buffer[256]{ };
    sprintf_s(buffer, "[%d] %s", m_Id, m_Name.c_str());
    return std::string(buffer);
}

std::uint32_t CProcessMemento::id() const {
    return m_Id;
}

const std::string& CProcessMemento::name() const {
    return m_Name;
}

const std::string& CProcessMemento::description() const {
    return m_Description;
}

CProcessList::CProcessList() {
    refresh();
}
CProcessList::~CProcessList() {
    cleanup();
}

void CProcessList::refresh() {
    cleanup();

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(!Utilities::isHandleValid(snapshot))
        return;

    PROCESSENTRY32 entry{ };
    entry.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(snapshot, &entry)) {
        do {
            std::wstring wName{ entry.szExeFile };
            m_Processes.emplace_back(static_cast<std::uint32_t>(entry.th32ProcessID), std::string(wName.begin(), wName.end()));
        } while(Process32Next(snapshot, &entry));
    }

    //sortByID();
    sortByName();
}

const std::vector<CProcessMemento>& CProcessList::data() const {
    return m_Processes;
}

void CProcessList::cleanup() {
    m_Processes.clear();
}

void CProcessList::sortByID() {
    std::sort(m_Processes.begin(), m_Processes.end());
}

void CProcessList::sortByName() {
    std::sort(m_Processes.begin(), m_Processes.end(), [](const CProcessMemento& p1, const CProcessMemento& p2) -> bool {
        return p1.name() < p2.name();
    });
}

CProcess::CProcess(std::uint32_t id)
    : CProcess(CProcessMemento(id, "")) { }

CProcess::CProcess(const CProcessMemento& process)
    : m_Memento{ process } {
    tryAttach();
    printf("Attaching: %s\n", m_Memento.name().c_str());
}

CProcess::~CProcess() {
    detach();
    printf("Detaching: %s\n", m_Memento.name().c_str());
}

HANDLE CProcess::handle() const {
    return m_Handle;
}

const CProcessMemento& CProcess::memento() const {
    return m_Memento;
}

bool CProcess::isAttached() const {
    return Utilities::isHandleValid(handle());
}

bool CProcess::tryAttach() {
    if(GetProcessId(GetCurrentProcess()) == m_Memento.id())
        return false;

    m_Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Memento.id());
    if(!isAttached())
        return false;

    return true;
}

void CProcess::detach() {
    if(!isAttached())
        return;

    CloseHandle(m_Handle);
    m_Handle = INVALID_HANDLE_VALUE;
}

bool IProcessIO::readPages(std::uint64_t startAddress, std::uint32_t size, std::uint8_t* buffer, std::uint8_t* invalidMask) {
    std::uint32_t remainingSize{ size }, offset{ 0 };
    int p{ }; // protect against deadloop
    while(remainingSize && ++p < 100) {
        std::uint64_t currentAddress = startAddress + offset;
        MBIEx mbi{ query(currentAddress) };
        if(!mbi.BaseAddress && !mbi.AllocationBase) {
            if(invalidMask) {
                memset(invalidMask, 1, size);
            }
            return false;
        }

        std::uint64_t pageEndAddress = reinterpret_cast<std::uint64_t>(mbi.BaseAddress) + mbi.RegionSize;
        std::uint32_t toReadSize = pageEndAddress - currentAddress;
        if(toReadSize > remainingSize)
            toReadSize = remainingSize;

        if(bool guarded = mbi.Protect & PAGE_GUARD; mbi.Protect & PAGE_NOACCESS || guarded) {
            for(std::size_t i = offset; i < toReadSize && invalidMask; ++i) {
                invalidMask[i] = guarded ? 2 : 1; // mark invalid bytes as "??" (like in windbg)
            }
            remainingSize -= toReadSize;
            offset += toReadSize;
            continue;
        }

        readToBuffer(currentAddress, toReadSize, buffer + offset);

        remainingSize -= toReadSize;
        offset += toReadSize;
    }
}
