#ifndef INC_2A03_APU_H
#define INC_2A03_APU_H

namespace NES {
class APU {
public:
    uint8_t read(uint8_t addr) {
        return 0x0;
    }

    void write(uint8_t val) {}
};

} // namespace NES

#endif
