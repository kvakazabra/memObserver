#include "dumper.h"

IDumper::IDumper(std::weak_ptr<IProcessIO> targetProcess)
    : m_TargetProcess{ targetProcess } { }

const std::vector<std::uint8_t>& IDumper::cachedDump() {
    if(m_Data.size())
        return m_Data;

    return dump();
}

CSectionDumper::CSectionDumper(std::weak_ptr<IProcessIO> targetProcess, std::uint64_t address, std::uint32_t size)
    : IDumper(targetProcess), m_Address{ address }, m_Size{ size } { }

const std::vector<std::uint8_t>& CSectionDumper::dump() {
    if(m_TargetProcess.expired() || !m_Size || !m_Address)
        return m_Data = { };

    std::vector<std::uint8_t> buffer(m_Size);
    if(!m_TargetProcess.lock()->readToBuffer(m_Address, m_Size, buffer.data()))
        return m_Data = { };

    return m_Data = std::move(buffer);
}

CModuleDumper::CModuleDumper(std::weak_ptr<IProcessIO> targetProcess, std::uint64_t address)
    : IDumper(targetProcess), m_Address{ address } { }

const std::vector<std::uint8_t>& CModuleDumper::dump() {
    if(m_TargetProcess.expired() || !m_Address)
        return m_Data = { };

    m_Module = std::make_unique<CModule>(CModuleMemento(m_Address, 0, ""), m_TargetProcess.lock().get());
    if(m_Module->sections().empty())
        return m_Data = { };

    // signatures are already checked by the CModule::parseSections()
    PIMAGE_DOS_HEADER dosHeader{ reinterpret_cast<PIMAGE_DOS_HEADER>(m_Module->headers().data()) };
    PIMAGE_NT_HEADERS64 ntHeaders{ reinterpret_cast<PIMAGE_NT_HEADERS64>(m_Module->headers().data() + dosHeader->e_lfanew) };
    m_Size = ntHeaders->OptionalHeader.SizeOfImage;

    m_Data = std::vector<std::uint8_t>(m_Size, 0);
    // check for allocated size?? idkidk

    // copy headers
    memcpy(m_Data.data(), m_Module->headers().data(), ntHeaders->OptionalHeader.SizeOfHeaders);

    for(auto& section : m_Module->sections()) {
        const auto& sectionHeader = section.rawInfo();
        if(!sectionHeader.VirtualAddress || !sectionHeader.PointerToRawData || !sectionHeader.Misc.VirtualSize) {
            // skip abnormal section
            continue;
        }

        // fix image by that time
        if(!m_TargetProcess.lock()->readToBuffer(m_Address + sectionHeader.VirtualAddress, sectionHeader.SizeOfRawData, m_Data.data() + sectionHeader.VirtualAddress))
            return m_Data = { };
    }

    fixSections();

    return m_Data;
}

void CModuleDumper::fixSections() {
    // signatures are already checked as this function must be called at the end of this->dump()
    PIMAGE_DOS_HEADER dosHeader{ reinterpret_cast<PIMAGE_DOS_HEADER>(m_Data.data()) };
    PIMAGE_NT_HEADERS64 ntHeaders{ reinterpret_cast<PIMAGE_NT_HEADERS64>(m_Data.data() + dosHeader->e_lfanew) };

    PIMAGE_SECTION_HEADER sections = IMAGE_FIRST_SECTION(ntHeaders);
    for(int i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
        sections[i].PointerToRawData = sections[i].VirtualAddress;
        sections[i].SizeOfRawData = sections[i].Misc.VirtualSize;
    }
}
