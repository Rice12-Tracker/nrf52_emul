#pragma once
#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>
#include <tinyxml2.h>

struct Register {
    std::string name;
    uint32_t address_offset;
};

struct Peripheral {
    std::string name;
    uint32_t base_address;
    std::unordered_map<uint32_t, Register> registers;
};

class SVDParser {
public:
    bool parse(const std::string& filename);
    const std::vector<Peripheral>& getPeripherals() const;
    const std::unordered_map<uint32_t, int>& getRegisters() const;

private:
    std::vector<Peripheral> peripherals_;
    std::unordered_map<uint32_t, int> registers_;
};