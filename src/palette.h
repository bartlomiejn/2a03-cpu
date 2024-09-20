#ifndef INC_2A03_PALETTE_H
#define INC_2A03_PALETTE_H

#include <stdint.h>

#include <cassert>
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

    std::vector<uint8_t> data;

    Palette() = delete;
    Palette(std::string filename) {
        std::ifstream ifs(filename, std::ios::binary);
        assert(ifs.is_open());
        std::copy(std::istream_iterator<uint8_t>(ifs),
                  std::istream_iterator<uint8_t>(), std::back_inserter(data));
    };

    Palette::Color get_rgb(uint8_t pal_6b) {
        return Color(data[pal_6b * 3], data[pal_6b * 3 + 1],
                     data[pal_6b * 3 + 2]);
    }
};

}  // namespace NES

#endif
