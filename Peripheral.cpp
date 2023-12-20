#include "Peripheral.h"

bool SVDParser::parse(const std::string& filename) {
    tinyxml2::XMLDocument doc;
    if (doc.LoadFile(filename.c_str()) != tinyxml2::XML_SUCCESS) {
        std::cerr << "Failed to load file: " << filename << std::endl;
        return false;
    }

    tinyxml2::XMLElement* device = doc.FirstChildElement("device");
    if (!device) {
        std::cerr << "Error: Missing 'device' element in XML." << std::endl;
        return false;
    }

    tinyxml2::XMLElement* peripherals = device->FirstChildElement("peripherals");
    if (!peripherals) {
        std::cerr << "Error: Missing 'peripherals' element under 'device'." << std::endl;
        return false;
    }
    
    std::unordered_map<std::string, Peripheral> peripheralMap;
    
    for (tinyxml2::XMLElement* peripheral = peripherals->FirstChildElement("peripheral");
         peripheral;
         peripheral = peripheral->NextSiblingElement("peripheral")) {

        Peripheral p;
        
        tinyxml2::XMLElement* nameElem = peripheral->FirstChildElement("name");
        if (!nameElem) {
            std::cerr << "Error: Missing 'name' element for a peripheral." << std::endl;
            continue;
        }
        p.name = nameElem->GetText();
        
        tinyxml2::XMLElement* baseAddressElem = peripheral->FirstChildElement("baseAddress");
        if (!baseAddressElem || baseAddressElem->QueryUnsignedText(&p.base_address) != tinyxml2::XML_SUCCESS) {
            std::cerr << "Error: Invalid 'baseAddress' for peripheral: " << p.name << std::endl;
            continue;
        }

        tinyxml2::XMLElement* registersElem = peripheral->FirstChildElement("registers");
        if (registersElem) {
            for (tinyxml2::XMLElement* reg = registersElem->FirstChildElement("register");
                 reg;
                 reg = reg->NextSiblingElement("register")) {

                Register r;

                tinyxml2::XMLElement* regNameElem = reg->FirstChildElement("name");
                if (!regNameElem) {
                    std::cerr << "Error: Missing 'name' element for a register under peripheral: " << p.name << std::endl;
                    continue;
                }
                r.name = regNameElem->GetText();

                tinyxml2::XMLElement* addressOffsetElem = reg->FirstChildElement("addressOffset");
                if (!addressOffsetElem || addressOffsetElem->QueryUnsignedText(&r.address_offset) != tinyxml2::XML_SUCCESS) {
                    std::cerr << "Error: Invalid 'addressOffset' for register: " << r.name << " under peripheral: " << p.name << std::endl;
                    continue;
                }

                p.registers[r.address_offset] = r;
            }
        }
        
        // Check for derivedFrom attribute
        const char* derivedFromAttr = peripheral->Attribute("derivedFrom");
        if (derivedFromAttr) {
            std::string basePeripheralName = derivedFromAttr;
            auto it = peripheralMap.find(basePeripheralName);
            if (it != peripheralMap.end()) {
                p.registers = it->second.registers; // Copy registers from base peripheral
            } else {
                std::cerr << "Error: Base peripheral " << basePeripheralName << " not found for derived peripheral: " << p.name << std::endl;
                continue;
            }
        }

        peripheralMap[p.name] = p; // Store the peripheral in the map
        
        
        int peripheralIndex = peripherals_.size();
        peripherals_.push_back(p);
        
        for (const auto& regPair : p.registers) {
            registers_[p.base_address+regPair.second.address_offset] = peripheralIndex;
        }
    }
    return true;
}

const std::vector<Peripheral>& SVDParser::getPeripherals() const {
    return peripherals_;
}

const std::unordered_map<uint32_t, int>& SVDParser::getRegisters() const {
    return registers_;
}