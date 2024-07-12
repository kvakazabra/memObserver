#pragma once
#include "process.h"

class CDumper {
public:
    CDumper(std::weak_ptr<IProcessIO> targetProcess);
    ~CDumper() = default;

    std::vector<std::uint8_t> dumpRegion(std::uint64_t address, std::uint32_t size);
    std::vector<std::uint8_t> dumpModule(std::uint64_t moduleBaseAddress);
private:
    std::weak_ptr<IProcessIO> m_TargetProcess{ };
};
