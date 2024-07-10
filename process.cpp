#include "process.h"

#include <TlHelp32.h>

CProcessMemento::CProcessMemento(CProcessMemento &&mv) {
    *this = std::move(mv);
};
CProcessMemento& CProcessMemento::operator=(CProcessMemento&& mv) {
    this->m_Id = mv.m_Id;
    this->m_Name = std::move(mv.m_Name);
    this->m_Description = std::move(mv.m_Description);
    return *this;
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
    if(snapshot == INVALID_HANDLE_VALUE)
        return;

    PROCESSENTRY32 entry{ };
    entry.dwSize = sizeof(PROCESSENTRY32);

    if(Process32First(snapshot, &entry)) {
        do {
            std::wstring wName{ entry.szExeFile };
            m_Processes.emplace_back(static_cast<std::uint32_t>(entry.th32ProcessID), std::string(wName.begin(), wName.end()));
        } while(Process32Next(snapshot, &entry));
    }
}

const std::vector<CProcessMemento>& CProcessList::data() const {
    return m_Processes;
}

void CProcessList::cleanup() {
    m_Processes.clear();
}
