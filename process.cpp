#include "process.h"

#include <TlHelp32.h>

CProcessMemento::CProcessMemento(const std::uint32_t id, const std::string& name)
    : m_Id{ id }
    , m_Name{ name } {

}
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

    sortByID();
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


CModuleMemento::CModuleMemento(const std::uint64_t baseAddress, const std::uint32_t size, const std::string& name)
    : m_BaseAddress{ baseAddress }, m_Size{ size }, m_Name{ name } {

}
CModuleMemento::CModuleMemento(CModuleMemento&& mv) {
    *this = std::move(mv);
}
CModuleMemento& CModuleMemento::operator=(CModuleMemento&& mv) {
    this->m_BaseAddress = mv.m_BaseAddress;
    this->m_Size = mv.m_Size;
    this->m_Name = std::move(mv.m_Name);
    return *this;
}
int operator<=>(const CModuleMemento& a1, const CModuleMemento& a2) {
    //return a1.m_Name - a2.m_Name;
    return a1.m_BaseAddress - a2.m_BaseAddress;
}

std::string CModuleMemento::format() const {
    char buffer[256]{ };
    sprintf_s(buffer, "%s", m_Name.c_str());
    return std::string(buffer);
}
std::tuple<std::uint64_t, std::uint32_t> CModuleMemento::info() const {
    return std::make_tuple(m_BaseAddress, m_Size);
}
const std::string& CModuleMemento::name() const {
    return m_Name;
}

CModuleList::CModuleList(std::weak_ptr<CProcess> process)
    : m_Process{ process } {
    refresh();
}
CModuleList::~CModuleList() {
    cleanup();
}
void CModuleList::refresh() {
    cleanup();

    if(m_Process.expired())
        return;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_Process.lock()->memento().id());
    if(!Utilities::isHandleValid(snapshot))
        return;

    MODULEENTRY32 entry{ };
    entry.dwSize = sizeof(MODULEENTRY32);

    if(Module32First(snapshot, &entry)) {
        do {
            std::wstring wName = std::wstring(entry.szModule);
            m_Modules.emplace_back(
                reinterpret_cast<std::uint64_t>(entry.modBaseAddr),
                static_cast<std::uint32_t>(entry.modBaseSize),
                std::string(wName.begin(), wName.end())
            );
        } while(Module32Next(snapshot, &entry));
    }

    sortByAddress();
}
const std::vector<CModuleMemento>& CModuleList::data() const {
    return m_Modules;
}
void CModuleList::cleanup() {
    m_Modules.clear();
}
void CModuleList::sortByAddress() {
    std::sort(m_Modules.begin(), m_Modules.end());
}
