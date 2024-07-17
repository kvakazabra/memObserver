#pragma once
#include <string>
#include <QString>
#include <Windows.h>
#include <stdexcept>
#include <array>
#include <fstream>
#include <filesystem>

class IFormattable {
public:
    virtual std::string format() const = 0;
};

template<class T>
class ISortStrategy {
public:
    virtual ~ISortStrategy() = default;

    virtual void sort(std::vector<T>& v) = 0;
};

template<class T>
class CNoSort : public ISortStrategy<T> {
public:
    void sort(std::vector<T>& v) override { };
};

class Utilities {
public:
    static bool isHandleValid(HANDLE h);
    static std::uint32_t processExitCode(HANDLE h);
    static bool isProcessActive(HANDLE h);
    static bool isProcessActive(std::uint32_t exitCode);
    static bool isValidASCIIChar(char c);

    static std::string generatePathForDump(const std::string& processName, const std::string& moduleName, const std::string& sectionName = "");
    static const std::string& programDataDirectory();
};

class MBIEx final : public MEMORY_BASIC_INFORMATION, public IFormattable {
public:
    MBIEx() = default;
    MBIEx(const MEMORY_BASIC_INFORMATION&a1);
public:
    virtual std::string format() const override;
};

class CBytesProtectionMask {
public:
    enum class TByteType : std::uint8_t {
        None,
        NoAccess,
        Guarded,
        //ReadOnly,
        Execute,
        RWX,
    };
public:
    CBytesProtectionMask(std::size_t size);

    CBytesProtectionMask(const CBytesProtectionMask&) = delete;
    CBytesProtectionMask& operator=(const CBytesProtectionMask&) = delete;
public:
    std::size_t size() const;
    TByteType& operator[](std::size_t index);
    const TByteType& operator[](std::size_t index) const;

    void setProtection(std::size_t index, std::uint32_t protection);
    void setAllProtection(std::uint32_t protection);
private:
    TByteType typeByProtection(std::uint32_t protection);

    std::vector<TByteType> m_Mask{ };
};

class CBytesProtectionMaskFormattablePlain : public CBytesProtectionMask {
public:
    CBytesProtectionMaskFormattablePlain(std::size_t size);

    std::string format(std::size_t index, std::uint8_t byte) const;
protected:
    std::string byteToString(std::uint8_t byte, CBytesProtectionMask::TByteType type = CBytesProtectionMask::TByteType::None) const;
};

class CBytesProtectionMaskFormattableHTML : public CBytesProtectionMaskFormattablePlain {
public:
    CBytesProtectionMaskFormattableHTML(std::size_t size);

    std::string format(std::size_t index, std::uint8_t byte) const;
};
