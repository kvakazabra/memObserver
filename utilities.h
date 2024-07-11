#pragma once
#include <string>
#include <QString>
#include <Windows.h>
#include <array>

class IFormattable {
public:
    virtual std::string format() const = 0;
    //virtual QString format() const = 0;
};

class Utilities {
public:
    static bool isHandleValid(HANDLE h);
};

class MBIEx final : public MEMORY_BASIC_INFORMATION, public IFormattable {
public:
    MBIEx() = default;
    MBIEx(const MEMORY_BASIC_INFORMATION&a1);
public:
    virtual std::string format() const override;
};
