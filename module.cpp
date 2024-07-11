#include "module.h"
#include "process.h"

#include <TlHelp32.h>

CSection::CSection(std::uint64_t baseAddress, std::uint32_t size, char* tag)
    : m_BaseAddress{ baseAddress }
    , m_Size{ size } {
    memcpy(m_Tag, tag, 8);
}

std::tuple<std::uint64_t, std::uint32_t> CSection::info() const {
    return std::make_tuple(m_BaseAddress, m_Size);
}
const char* CSection::tag() const {
    return m_Tag;
}

CModuleMemento::CModuleMemento(const std::uint64_t baseAddress, const std::uint32_t size, const std::string& name)
    : m_BaseAddress{ baseAddress }, m_Size{ size }, m_Name{ name } { }

CModuleMemento::CModuleMemento(CModuleMemento&& mv) noexcept {
    *this = std::move(mv);
}

CModuleMemento& CModuleMemento::operator=(CModuleMemento&& mv) noexcept {
    this->m_BaseAddress = mv.m_BaseAddress;
    this->m_Size = mv.m_Size;
    this->m_Name = std::move(mv.m_Name);
    return *this;
}

int operator<=>(const CModuleMemento& a1, const CModuleMemento& a2) {
    return a1.m_BaseAddress - a2.m_BaseAddress;
}

std::string CModuleMemento::format() const {
    char buffer[MAX_PATH]{ };
    sprintf_s(buffer, "%s", m_Name.c_str());
    return std::string(buffer);
}

std::tuple<std::uint64_t, std::uint32_t> CModuleMemento::info() const {
    return std::make_tuple(m_BaseAddress, m_Size);
}

const std::string& CModuleMemento::name() const {
    return m_Name;
}

CModule::CModule(const CModuleMemento& module, IProcessIO* process)
    : m_Memento{ module }, m_ThisProcess{ process } {
    if(!m_ThisProcess)
        throw std::runtime_error("m_ThisProcess can not be a nullptr");

    //printf("[CModule] Instantiated %s module\n", m_Memento.name().c_str());
    parseSections();
}

const std::vector<CSection>& CModule::sections() const {
    return m_Sections;
}

void CModule::parseSections() {
    auto [baseAddress, size] = memento().info();

    auto buffer{ std::make_unique_for_overwrite<std::uint8_t[]>(0x1000) };
    if(!buffer)
        return;

    if(!m_ThisProcess->readToBuffer(baseAddress, 0x1000, buffer.get()))
        return;

    PIMAGE_DOS_HEADER dosHeader{ reinterpret_cast<PIMAGE_DOS_HEADER>(buffer.get()) };
    if(dosHeader->e_magic != 0x5a4d) // MZ signature
        return;

    PIMAGE_NT_HEADERS64 ntHeaders{ reinterpret_cast<PIMAGE_NT_HEADERS64>(buffer.get() + dosHeader->e_lfanew) };
    if(ntHeaders->Signature != 0x4550) // PE signature
        return;

    PIMAGE_SECTION_HEADER sections{ IMAGE_FIRST_SECTION(ntHeaders) };
    for(int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        IMAGE_SECTION_HEADER section = sections[i];
        std::uint64_t sectionBaseAddress = baseAddress + section.VirtualAddress;
        m_Sections.emplace_back(sectionBaseAddress, section.SizeOfRawData, reinterpret_cast<char*>(section.Name));
    }
}

const CModuleMemento& CModule::memento() const {
    return m_Memento;
}
