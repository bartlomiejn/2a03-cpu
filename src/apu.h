#ifndef INC_2A03_APU_H
#define INC_2A03_APU_H

#include <cstdint>

namespace NES {
class APU {
   public:
    uint8_t read(uint8_t addr) { return 0xFF; }

    void write(uint8_t val) {}
};

}  // namespace NES

#endif
