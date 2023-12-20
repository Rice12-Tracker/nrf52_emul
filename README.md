# nrf52_emul
nRF52 emulation tool with Unicorn Engine
# Setting up Environment
```
git clone https://github.com/unicorn-engine/unicorn.git
cd unicorn
mkdir build; cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
sudo make install # install Unicorn Engine globally
sudo apt-get install libtinyxml2-dev
```
# Code Detail
1. Peripheral.h/Peripheral.cpp: SVD parser & Peripheral/Register struct
2. Emulator.h/Emulator.cpp: Emulator class
3. main.cpp: Emulation with firmware binary & SVD file

- fix Emulator::init() for set initial status.
- fix main.cpp for set firmware binary & SVD file
