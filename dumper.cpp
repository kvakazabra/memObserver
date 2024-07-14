#include "dumper.h"

IDumper::IDumper(std::weak_ptr<IProcessIO> targetProcess)
    : m_TargetProcess{ targetProcess } { }

const std::vector<std::uint8_t>& IDumper::cachedDump() const {
    if(m_Data.size())
        return m_Data;

    return dump();
}

CSectionDumper::CSectionDumper(std::weak_ptr<IProcessIO> targetProcess, std::uint64_t address, std::uint32_t size)
    : IDumper{ targetProcess }, m_Address{ address }, m_Size{ size } {
    // if(!m_Size || !m_Address)
    //     throw std::runtime_error("CSectionDumper: m_Address or m_Size can not be null");
}

const std::vector<std::uint8_t>& CSectionDumper::dump() const {
    if(m_TargetProcess.expired() || !m_Size || !m_Address)
        return { };

    std::vector<std::uint8_t> buffer(m_Size);
    if(!m_TargetProcess.lock()->readToBuffer(m_Address, m_Size, buffer.data()))
        return { };

    return m_Data = std::move(buffer);
}
