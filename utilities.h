#pragma once
#include <string>
#include <QString>

class IFormattable {
public:
    virtual std::string format() const = 0;
    //virtual QString format() const = 0;
};
