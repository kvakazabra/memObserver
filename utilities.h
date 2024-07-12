#pragma once
#include <string>
#include <QString>
#include <Windows.h>
#include <array>
#include <stdexcept>

class IFormattable {
public:
    virtual std::string format() const = 0;
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

    std::size_t m_Size{ };
    std::unique_ptr<TByteType[]> m_Mask{ };
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
