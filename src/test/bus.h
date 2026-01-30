#ifndef INC_2A03_TEST_BUS_H
#define INC_2A03_TEST_BUS_H

#include <bus.h>
#include <log.h>

#include <format>

namespace NES {

namespace Test {

struct BusAccess {
    uint16_t addr;
    uint8_t val;
    bool read; 
};

class MemoryBus : public MemoryBusIntf {
public:
    static const int ram_size = 65536;
    std::array<uint8_t, ram_size> ram{};
    std::vector<BusAccess> ops;

    ~MemoryBus() = default;

    uint8_t read(uint16_t addr, bool passive) {
        NES_LOG("Bus") << std::format(
            "mock_bus read@{:04X}={:02X} passive={}\n", addr, ram[addr],
            passive);
        if (!passive) {
            ops.push_back(BusAccess(addr, ram[addr], true));
        }
        return ram[addr];
    }

    void write(uint16_t addr, uint8_t val) {
        NES_LOG("Bus") << std::format("mock_bus write@{:04X} {:02X}\n", addr,
                                      val);
        ops.push_back(BusAccess(addr, val, false));
        ram[addr] = val;
    }

    void mock_clear_ops() {
        ops.clear();
    }

    uint8_t mock_read(uint16_t addr) {
        return ram[addr];
    }

    void mock_write(uint16_t addr, uint8_t val) {
        ram[addr] = val;
    }
};

}

}

#endif
