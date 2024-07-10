#pragma once
#include <string>
#include <QString>
#include <Windows.h>

class IFormattable {
public:
    virtual std::string format() const = 0;
    //virtual QString format() const = 0;
};

class Utilities {
public:
    static bool isHandleValid(HANDLE h);
};
