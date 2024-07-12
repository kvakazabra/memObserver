#include "dumper.h"

CDumper::CDumper(std::weak_ptr<IProcessIO> targetProcess)
    : m_TargetProcess{ targetProcess } { }

std::vector<std::uint8_t> CDumper::dumpRegion(std::uint64_t address, std::uint32_t size) {
    if(m_TargetProcess.expired())
        return { };

    std::vector<std::uint8_t> buffer(size);
    if(!m_TargetProcess.lock()->readToBuffer(address, size, buffer.data()))
        return { };

    return buffer;
}
