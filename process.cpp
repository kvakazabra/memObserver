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
    : m_Id{ process.id() }
    , m_Name{ process.name() } {
    tryAttach();
    printf("Attaching: %s\n", m_Name.c_str());
}
CProcess::~CProcess() {
    detach();
    printf("Detaching: %s\n", m_Name.c_str());
}

HANDLE CProcess::handle() const {
    return m_Handle;
}
const std::string& CProcess::name() const {
    return m_Name;
}
std::uint32_t CProcess::id() const {
    return m_Id;
}

bool CProcess::isAttached() const {
    return Utilities::isHandleValid(handle());
}
bool CProcess::tryAttach() {
    m_Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, m_Id);
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
