#include "utilities.h"
#include <shlobj_core.h>

bool Utilities::isHandleValid(HANDLE h) {
    return h != 0 && h != INVALID_HANDLE_VALUE;
}

std::uint32_t Utilities::processExitCode(HANDLE h) {
    DWORD exitCode{ };
    if(!GetExitCodeProcess(h, &exitCode)) // not sure about this one
        return UINT_MAX;
    return exitCode;
}

bool Utilities::isProcessActive(HANDLE h) {
    const auto exitCode = processExitCode(h);
    if(exitCode == UINT_MAX) // error in processExitCode
        return false;

    return isProcessActive(exitCode);
}

bool Utilities::isProcessActive(std::uint32_t exitCode) {
    return exitCode == STILL_ACTIVE;
}

bool Utilities::isValidASCIIChar(char c) {
    return c > 0x20 && c < 0x7f;
}

std::string Utilities::generatePathForDump(const std::string& processName, const std::string& moduleName, const std::string& sectionName) {
    if(sectionName.empty())
        return programDataDirectory() + std::string("\\") +
               processName + std::string("_") +
               moduleName + std::string("_") +
               std::to_string(time(NULL)) + std::string(".dmp");
    else
        return programDataDirectory() + std::string("\\") +
               moduleName + std::string("_") +
               sectionName + std::string("_") +
               std::to_string(time(NULL)) + std::string(".dmp");
}

const std::string& Utilities::programDataDirectory() {
    static std::string path{ };
    if(!path.empty())
        return path;

    char buffer[MAX_PATH] = { };
    if (!SUCCEEDED(SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, SHGFP_TYPE_CURRENT, buffer))) {
        throw std::runtime_error("Critical: SHGetFolderPathW failed when getting program data directory");
    }

    path = std::string(buffer) + std::string("\\memObserver");
    std::filesystem::create_directories(path);

    return path;
}

MBIEx::MBIEx(const MEMORY_BASIC_INFORMATION& a1)
    : MEMORY_BASIC_INFORMATION{ a1 } { }

std::string MBIEx::format() const {
    static std::unordered_map<int, const std::string> protectionTranslations{
        { PAGE_NOACCESS, "No Access" },
        { PAGE_EXECUTE, "X" },
        { PAGE_EXECUTE_READ, "RX" },
        { PAGE_EXECUTE_READWRITE, "RWX" },
        { PAGE_EXECUTE_WRITECOPY, "RWX Copy" },
        { PAGE_READONLY, "Read Only" },
        { PAGE_READWRITE, "RW" },
        { PAGE_WRITECOPY, "RW Copy" },
        { PAGE_TARGETS_INVALID, "PAGE_TARGETS_INVALID" },
        { PAGE_TARGETS_NO_UPDATE, "PAGE_TARGETS_NO_UPDATE" },
        { PAGE_GUARD, "Guarded" },
        { PAGE_NOCACHE, "No Cache" },
        { PAGE_WRITECOMBINE, "Write Combine" },
        { PAGE_ENCLAVE_DECOMMIT, "PAGE_ENCLAVE_DECOMMIT" },
        { PAGE_ENCLAVE_THREAD_CONTROL, "PAGE_ENCLAVE_THREAD_CONTROL" },
        { PAGE_ENCLAVE_UNVALIDATED, "PAGE_ENCLAVE_UNVALIDATED" },
    };

    static const std::array<DWORD, 16> protections{
        PAGE_NOACCESS, PAGE_EXECUTE, PAGE_EXECUTE_READ, PAGE_EXECUTE_READWRITE,
        PAGE_EXECUTE_WRITECOPY, PAGE_READONLY, PAGE_READWRITE, PAGE_WRITECOPY,
        PAGE_TARGETS_INVALID, PAGE_TARGETS_NO_UPDATE, PAGE_GUARD, PAGE_NOCACHE,
        PAGE_WRITECOMBINE, PAGE_ENCLAVE_DECOMMIT, PAGE_ENCLAVE_THREAD_CONTROL, PAGE_ENCLAVE_UNVALIDATED,
    }; // there's more protections such as PAGE_GRAPHICS_... but in past they didn't work for me, limited use

    std::string protectionsString{ };
    for(auto& p : protections) {
        if(!(this->Protect & p))
            continue;

        protectionsString += protectionTranslations[p];
        protectionsString += " ";
    }

    return protectionsString;
}

CBytesProtectionMask::CBytesProtectionMask(std::size_t size)
    : m_Mask{ std::vector<CBytesProtectionMask::TByteType>(size, CBytesProtectionMask::TByteType::None) } { }

std::size_t CBytesProtectionMask::size() const {
    return m_Mask.size();
}

CBytesProtectionMask::TByteType& CBytesProtectionMask::operator[](std::size_t index) {
    if(index >= m_Mask.size())
        throw std::out_of_range("CBytesProtectionMask operator[] out of range");

    return m_Mask[index];
}

const CBytesProtectionMask::TByteType& CBytesProtectionMask::operator[](std::size_t index) const {
    if(index >= m_Mask.size())
        throw std::out_of_range("CBytesProtectionMask const operator[] out of range");

    return m_Mask[index];
}

CBytesProtectionMask::TByteType CBytesProtectionMask::typeByProtection(std::uint32_t protection) {
    CBytesProtectionMask::TByteType result{ CBytesProtectionMask::TByteType::None };
    if(protection & PAGE_EXECUTE || protection & PAGE_EXECUTE_READ)
        result = CBytesProtectionMask::TByteType::Execute;
    if(protection & PAGE_EXECUTE_READWRITE)
        result = CBytesProtectionMask::TByteType::RWX;
    if(protection & PAGE_NOACCESS)
        result = CBytesProtectionMask::TByteType::NoAccess;
    if(protection & PAGE_GUARD)
        result = CBytesProtectionMask::TByteType::Guarded;

    return result;
}

void CBytesProtectionMask::setProtection(std::size_t index, std::uint32_t protection) {
    operator[](index) = typeByProtection(protection);
}

void CBytesProtectionMask::setAllProtection(std::uint32_t protection) {
    const auto type = typeByProtection(protection);
    for(std::size_t i = 0; i < m_Mask.size(); ++i) {
        operator[](i) = type;
    }
}

CBytesProtectionMaskFormattablePlain::CBytesProtectionMaskFormattablePlain(std::size_t size)
    :CBytesProtectionMask(size) { }

std::string CBytesProtectionMaskFormattablePlain::format(std::size_t index, std::uint8_t byte) const {
    return byteToString(byte, operator[](index));
}

std::string CBytesProtectionMaskFormattablePlain::byteToString(std::uint8_t byte, CBytesProtectionMask::TByteType type) const {
    static char invalidByte[4]{"??"}, guardedByte[4]{"xx"};

    if(type == CBytesProtectionMask::TByteType::Guarded)
        return std::string(guardedByte);
    if(type == CBytesProtectionMask::TByteType::NoAccess)
        return std::string(invalidByte);

    char buffer[4]{ };
    sprintf_s(buffer, "%02x", byte);
    return std::string(buffer);
}

CBytesProtectionMaskFormattableHTML::CBytesProtectionMaskFormattableHTML(std::size_t size)
    : CBytesProtectionMaskFormattablePlain(size) { }

std::string CBytesProtectionMaskFormattableHTML::format(std::size_t index, std::uint8_t byte) const {
    static std::unordered_map<CBytesProtectionMask::TByteType, const std::string> protectionColors{
        { CBytesProtectionMask::TByteType::NoAccess, "<font color=\"LightSlateGray\">" },
        { CBytesProtectionMask::TByteType::Guarded, "<font color=\"Navy\">" },
        { CBytesProtectionMask::TByteType::RWX, "<font color=\"Indigo\">" },
        { CBytesProtectionMask::TByteType::Execute, "<font color=\"OliveDrab\">" },
    };

    static const std::string styleEnd{ "</font>" };

    CBytesProtectionMask::TByteType type{ operator[](index) };
    if(type == CBytesProtectionMask::TByteType::None)
        return byteToString(byte);

    return protectionColors[type] + byteToString(byte, type) + styleEnd;
}
