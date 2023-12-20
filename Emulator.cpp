#include "Emulator.h"


void Emulator::uc_c_cb(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
    Emulator* self = static_cast<Emulator*>(user_data);
    if (self) {
        self->uc_c_callback(uc, address, size);
    }
}    
void Emulator::uc_rw_cb(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int32_t value, void *user_data) {
    Emulator* self = static_cast<Emulator*>(user_data);
    if (self) {
        self->uc_rw_callback(uc, type, address, size, value);
    }
}
void Emulator::uc_intr_cb(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
    Emulator* self = static_cast<Emulator*>(user_data);
    if (self) {
        self->uc_intr_callback(uc, address, size);
    }
}
void Emulator::uc_exc_cb(uc_engine *uc, uint64_t address, uint32_t size, void *user_data) {
    Emulator* self = static_cast<Emulator*>(user_data);
    if (self) {
        self->uc_exc_callback(uc, address, size);
    }
}

void Emulator::uc_rw_callback(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int32_t value) {
    auto it = registers.find(address);
    if (it == registers.end()) {
        return;  // Address not in our registers map
    }

    int peripheralIndex = it->second;
    if (peripheralIndex >= 0 && peripheralIndex < peripherals.size()) {
        const Peripheral& p = peripherals[peripheralIndex];
        auto regIt = p.registers.find(address - p.base_address); // Adjust the address to get the offset

        if (regIt == p.registers.end()) {
            return;  // This shouldn't happen if our map is correct
        }

        const Register& r = regIt->second;

        switch (type) {
        default:
            break;
        case UC_MEM_READ:
            std::cout << ">>> Memory (Register: " << r.name << " of Peripheral: " << p.name << ") is being READ at 0x" 
                      << std::hex << address << std::dec << ", data size = " << size << std::endl;
            break;
        case UC_MEM_WRITE:
            std::cout << ">>> Memory (Register: " << r.name << " of Peripheral: " << p.name << ") is being WRITE at 0x" 
                      << std::hex << address << std::dec << ", data size = " << size << ", data value = 0x" 
                      << std::hex << value << std::dec << std::endl;
            break;
        }
    }
}
void Emulator::uc_c_callback(uc_engine *uc, uint64_t address, uint32_t size) {
    
}
void Emulator::uc_intr_callback(uc_engine *uc, uint64_t address, uint32_t size) {
    
}
void Emulator::uc_exc_callback(uc_engine *uc, uint64_t address, uint32_t size) {
    
}
Emulator::Emulator() : uc(nullptr) {
    // Memory map initialization
    memoryMaps = {
        {0x20000000, 0x40000, UC_PROT_ALL},
        {0xf0000000, 0x1000 , UC_PROT_READ | UC_PROT_WRITE},
        {0xe0000000, 0x10000, UC_PROT_READ | UC_PROT_WRITE},
        {0x10000000, 0x10000, UC_PROT_READ | UC_PROT_WRITE},
        {0x40000000, 0x40000, UC_PROT_READ | UC_PROT_WRITE},
        {0x50000000, 0x1000 , UC_PROT_READ | UC_PROT_WRITE},
        {0xfffff000, 0x1000 , UC_PROT_READ | UC_PROT_WRITE}
    };
}

Emulator::~Emulator() {
    if (uc) uc_close(uc);
}


bool Emulator::loadFirmware(const std::string &filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    
    if (!file.is_open()) {
        return false;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    firmwareBuffer.resize(size);
    if (!file.read(reinterpret_cast<char*>(firmwareBuffer.data()), size)) {
        return false;
    }
    return true;
}

bool Emulator::loadSVD(const std::string &filePath) {
    SVDParser parser;
    
    if (!parser.parse(filePath)) {
        std::cerr << "Error: Failed to parse the SVD file" << std::endl;
        return false;
    }
    
    peripherals = parser.getPeripherals();
    if (peripherals.empty()) {
        std::cerr << "Warning: No peripherals found in the parsed SVD file." << std::endl;
        return false;
    }

    registers = parser.getRegisters();
    if (registers.empty()) {
        std::cerr << "Warning: No registers found in the parsed SVD file." << std::endl;
    }
    
    return true;
}


uint32_t Emulator::getVectorValue(VectorTable vt) {
    if (vt >= 0 && vt < 15 && !firmwareBuffer.empty()) {
        return *reinterpret_cast<uint32_t*>(firmwareBuffer.data() + vt * sizeof(uint32_t));
    }
    throw std::runtime_error("Invalid vector table index or firmware not loaded.");
}

bool Emulator::init(const std::string &Fimwarefilename,const std::string &SVDfilename) {
    // ARM Cortex-M
    if (uc_open(UC_ARCH_ARM, (uc_mode)(UC_MODE_LITTLE_ENDIAN|UC_MODE_MCLASS), &uc) != UC_ERR_OK) {
        std::cerr << "Failed to initialize Unicorn engine." << std::endl;
        return false;
    }
    // Load Firmware
    if (!loadFirmware(Fimwarefilename)) {
        std::cerr << "Failed to load firmware." << std::endl;
        return false;
    }
    // Load SVD
    if (!loadSVD(SVDfilename)) {
        std::cerr << "Error: Failed to load the SVD file." << std::endl;
        return false;
    }
    
    // Map&Write firmware
    if (uc_mem_map(uc, 0, firmwareBuffer.size(), UC_PROT_ALL) != UC_ERR_OK) {
        std::cerr << "Failed to map memory at address " << std::hex << 0 << "." << std::endl;
        return -1;
    }
    if (uc_mem_write(uc, 0, firmwareBuffer.data(), firmwareBuffer.size()) != UC_ERR_OK) {
        std::cerr << "Failed to write firmware to emulated memory." << std::endl;
        return -1;
    }
    // Map memory
    for (const auto &mapInfo : memoryMaps) {
        uint64_t startAddress = std::get<0>(mapInfo);
        uint64_t size = std::get<1>(mapInfo);
        int protection = std::get<2>(mapInfo);

        if (uc_mem_map(uc, startAddress, size, protection) != UC_ERR_OK) {
            std::cerr << "Failed to map memory at address " << std::hex << startAddress << "." << std::endl;
            return -1;
        }
    }
    // Add hooks
    uc_hook hook_c, hook_rw, hook_intr, hook_exc_ret;

    uc_hook_add(uc, &hook_c, UC_HOOK_CODE, reinterpret_cast<void *>(uc_c_cb), nullptr, 1, 0);
    uc_hook_add(uc, &hook_rw, UC_HOOK_MEM_READ | UC_HOOK_MEM_WRITE, reinterpret_cast<void*>(Emulator::uc_rw_cb), this, 1, 0);
    uc_hook_add(uc, &hook_intr, UC_HOOK_INTR, reinterpret_cast<void *>(uc_intr_cb), nullptr, 1, 0);
    uc_hook_add(uc, &hook_exc_ret, UC_HOOK_BLOCK, reinterpret_cast<void *>(uc_exc_cb), nullptr, 0xfffff000, 0xffffffff);
    
    // Set NRFFW register
    uint32_t temp=-1;
    uc_mem_write(uc, 0x10001014, &temp, 4);
    // Set MSP
    uint32_t initial_sp = getVectorValue(Emulator::VT_INITIAL_SP);
    uc_reg_write(uc,UC_ARM_REG_MSP, &initial_sp);
    // set PC
    uint32_t reset_handler = getVectorValue(Emulator::VT_RESET);
    uc_reg_write(uc,UC_ARM_REG_PC, &reset_handler);
    
    return true;
}

bool Emulator::run(int timeout) {    
    // Start emulation
    uint32_t reset_handler = getVectorValue(Emulator::VT_RESET);
    if (uc_emu_start(uc, reset_handler, firmwareBuffer.size(), timeout * UC_SECOND_SCALE, 0) != UC_ERR_OK) {
        std::cerr << "Failed on uc_emu_start()" << std::endl;
        return false;
    }
    return true;
}