#pragma once

#include <iostream>
#include <tuple>
#include <vector>
#include <fstream>
#include <unicorn/unicorn.h>
#include <functional>
#include "Peripheral.h"

class Emulator {
private:
    uc_engine *uc;
    
    std::vector<uint8_t> firmwareBuffer;
    std::vector<Peripheral> peripherals;
    std::unordered_map<uint32_t, int> registers;
    
    // Memory map
    using MemoryMapTuple = std::tuple<uint64_t, uint64_t, int>;
    std::vector<MemoryMapTuple> memoryMaps;
    // Vector table
    enum VectorTable {
        VT_INITIAL_SP,
        VT_RESET,
        VT_NMI,
        VT_HARD_FAULT,
        VT_MEM_MANAGE,
        VT_BUS_FAULT,
        VT_USAGE_FAULT,
        VT_RESERVED_1,
        VT_RESERVED_2,
        VT_RESERVED_3,
        VT_SVC,
        VT_DEBUG_MONITOR,
        VT_RESERVED_4,
        VT_PEND_SV,
        VT_SYSTICK,
        VT_IRQ
    };
    
    bool loadFirmware(const std::string &filePath);
    bool loadSVD(const std::string &filePath);
    uint32_t getVectorValue(VectorTable vt);
    // callback bridges
    static void uc_c_cb(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
    static void uc_rw_cb(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int32_t value, void *user_data);
    static void uc_intr_cb(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
    static void uc_exc_cb(uc_engine *uc, uint64_t address, uint32_t size, void *user_data);
    // callbacks
    void uc_c_callback(uc_engine *uc, uint64_t address, uint32_t size);
    void uc_rw_callback(uc_engine *uc, uc_mem_type type, uint64_t address, uint32_t size, int32_t value);
    void uc_intr_callback(uc_engine *uc, uint64_t address, uint32_t size);
    void uc_exc_callback(uc_engine *uc, uint64_t address, uint32_t size);
    
public:
    Emulator();
    ~Emulator();
    
    bool init(const std::string &Fimwarefilename,const std::string &SVDfilename);
    bool run(int timeout);
};