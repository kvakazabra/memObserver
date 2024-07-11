#include "module.h"

#include <TlHelp32.h>

CModuleMemento::CModuleMemento(const std::uint64_t baseAddress, const std::uint32_t size, const std::string& name)
    : m_BaseAddress{ baseAddress }, m_Size{ size }, m_Name{ name } { }

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

    //sortByAddress();
    sortByName();
}

const std::vector<CModuleMemento>& CModuleList::data() const {
    return m_Modules;
}

void CModuleList::cleanup() {
    m_Modules.clear();
}

void CModuleList::sortByName() {
    std::sort(m_Modules.begin(), m_Modules.end(), [](const CModuleMemento& m1, const CModuleMemento& m2) -> bool {
        return m1.name() < m2.name();
    });
}

void CModuleList::sortByAddress() {
    std::sort(m_Modules.begin(), m_Modules.end());
}
