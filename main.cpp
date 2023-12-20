#include "Emulator.h"
#include "Peripheral.h"

int main() {
    Emulator emu;
    if (!emu.init("data/flash.bin","data/nrf52832.svd")) return -1;
    //if (!emu.init("data/FLASH_smarttag.bin","data/nrf52833.svd")) return -1;
    if (!emu.run(2)) return -1;
    return 0;
}