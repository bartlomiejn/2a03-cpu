#ifndef INC_2A03_APU_H
#define INC_2A03_APU_H

#include <log.h>

#include <cstdint>
#include <format>

namespace NES {
class APU {
   public:
    uint8_t read(uint8_t addr) {
        NES_LOG("APU") << std::format("read@40{:02X}=00 dummy read\n", addr);
        return 0x00;
    }

    void write(uint8_t addr, uint8_t val) {
        NES_LOG("APU") << std::format("write@40{:02X} {:02X} dummy write\n",
                                      addr, val);
    }
};

}  // namespace NES

#endif
