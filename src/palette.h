#ifndef INC_2A03_PALETTE_H
#define INC_2A03_PALETTE_H

#include <log.h>
#include <stdint.h>

#include <cassert>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace NES {

class Palette {
   public:
    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    };

    std::array<uint8_t, 0xC0> data;

    Palette() = delete;
    explicit Palette(std::string filename) {
        std::ifstream ifs(filename, std::ios::binary);
        assert(ifs.is_open());
        ifs.read(reinterpret_cast<char*>(data.data()), data.size());
    };

    uint32_t get_rgba(uint8_t idx) {
        uint8_t offset = idx * 3;
        uint32_t retval = (data[offset] << 24) | (data[offset + 1] << 16) |
                          (data[offset + 2] << 8) | 0xFF;
        return retval;
    }
};

}  // namespace NES

#endif
