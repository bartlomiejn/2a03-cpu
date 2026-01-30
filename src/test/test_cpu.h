#ifndef INC_2A03_TEST_CPU_H
#define INC_2A03_TEST_CPU_H

#include <json.hpp>
#include <iostream>
#include <fstream>
#include <format>
#include <cassert>

namespace NES {

namespace Test {

using json = nlohmann::json;

struct RAMState {
    int address;
    int value;
};

struct NESState {
    int pc;
    int s;
    int a;
    int x;
    int y;
    int p;
    std::vector<RAMState> ram;
};

struct MemoryAccessCycle {
    int address;
    int value;
    std::string operation;
};

struct TestCase {
    std::string name;
    NESState initial;
    NESState final;
    std::vector<MemoryAccessCycle> cycles;
};

void from_json(const json& j, RAMState& r) {
    r.address = j.at(0).get<int>();
    r.value = j.at(1).get<int>();
}

void from_json(const json& j, NESState& s) {
    s.pc = j.at("pc").get<int>();
    s.s = j.at("s").get<int>();
    s.a = j.at("a").get<int>();
    s.x = j.at("x").get<int>();
    s.y = j.at("y").get<int>();
    s.p = j.at("p").get<int>();
    s.ram = j.at("ram").get<std::vector<RAMState>>();
}

void from_json(const json& j, MemoryAccessCycle& c) {
    c.address = j.at(0).get<int>();
    c.value = j.at(1).get<int>();
    c.operation = j.at(2).get<std::string>();
}

void from_json(const json& j, TestCase& d) {
    d.name = j.at("name").get<std::string>();
    d.initial = j.at("initial").get<NESState>();
    d.final = j.at("final").get<NESState>();
    d.cycles = j.at("cycles").get<std::vector<MemoryAccessCycle>>();
}

std::string get_testspec_name(uint8_t index) {
    return std::format("nes6502/{:02x}.json", static_cast<unsigned int>(index));
}

void print_test_case(const TestCase &tc, std::ostream &os) {
    os << "FAILED ON TEST CASE: " << tc.name << "\n";
    os << std::format(
        "Initial CPU: PC=0x{:04X} S=0x{:02X} A=0x{:02X} "
        "X=0x{:02X} Y={:02X} P={:02X}\n",
        tc.initial.pc, tc.initial.s, tc.initial.a, tc.initial.x, tc.initial.y,
        tc.initial.p);

    os << "Initial RAM:" << "\n";
    for (const auto &ram : tc.initial.ram) {
        os << std::format("Address: 0x{:04X} Value: 0x{:02X}\n", ram.address,
                          ram.value);
    }

    os << std::format(
        "Final CPU: PC=0x{:04X} S=0x{:02X} A=0x{:02X} "
        "X=0x{:02X} Y={:02X} P={:02X}\n",
        tc.final.pc, tc.final.s, tc.final.a, tc.final.x, tc.final.y,
        tc.final.p);

    os << "Final RAM:" << "\n";
    for (const auto &ram : tc.final.ram) {
        os << std::format("Address: 0x{:04X} Value: 0x{:02X}\n", ram.address,
                          ram.value);
    }

    os << "Memory accesses:" << "\n";
    for (const auto &cycle : tc.cycles) {
        os << std::format(
            "Addr: 0x{:04X} Value: 0x{:02X} Operation: "
            "{}\n",
            cycle.address, cycle.value, cycle.operation);
    }
}

void print_actual_state(NES::ExecutionEnvironment &ee,
                        NES::Test::MemoryBus *bus, const TestCase &tc,
                        std::ostream &os) {
    os << "ACTUAL STATE" << std::endl;

    os << std::format(
        "Actual CPU: PC=0x{:04X} S=0x{:02X} A=0x{:02X} "
        "X=0x{:02X} Y={:02X} P={:02X}\n",
        ee.cpu.PC, ee.cpu.S, ee.cpu.A, ee.cpu.X, ee.cpu.Y, ee.cpu.P.status);

    os << "Actual RAM:" << std::endl;
    for (const auto &ram : tc.final.ram) {
        os << std::format("Address: 0x{:04X} Value: 0x{:02X}\n", ram.address,
                          bus->mock_read(ram.address));
    }

    os << "Actual memory accesses:" << std::endl;
    for (const auto &op : bus->ops) {
        os << std::format(
            "Addr: 0x{:04X} Value: 0x{:02X} Operation: "
            "{}\n",
            op.addr, op.val, (op.read ? "read" : "write"));
    }
}

template<typename T, typename U>
bool assert_equal(const T& a, const U& b, const std::string& a_str, 
                  const std::string& b_str) {
    if (!(a == b)) {
        std::cerr << std::format("\nASSERTION FAILED: {} == {}\n", a_str, b_str)
                  << std::format("  {} = 0x{:02X}\n", a_str, a)
                  << std::format("  {} = 0x{:02X}\n", b_str, b);
        return false;
    } 
    return true;
}

#define ASSERT_EQUAL(a, b)                                   \
    if (!assert_equal(a, b, #a, #b)) {                       \
        print_test_case(tc, std::cout);                      \
        print_test_case(tc, NES_LOG("cputest"));             \
        print_actual_state(ee, bus, tc, std::cout);          \
        print_actual_state(ee, bus, tc, NES_LOG("cputest")); \
        return false;                                        \
    }

bool assert_final_state(NES::ExecutionEnvironment &ee,
                        NES::Test::MemoryBus *bus, const TestCase &tc) {
    ASSERT_EQUAL(ee.cpu.PC, tc.final.pc);
    ASSERT_EQUAL(ee.cpu.S, tc.final.s);
    ASSERT_EQUAL(ee.cpu.A, tc.final.a);
    ASSERT_EQUAL(ee.cpu.X, tc.final.x);
    ASSERT_EQUAL(ee.cpu.Y, tc.final.y);
    ASSERT_EQUAL(ee.cpu.P.status, tc.final.p);

    for (const auto &ram : tc.final.ram) {
        uint8_t actual_value = bus->mock_read((uint16_t)ram.address);
        ASSERT_EQUAL(ram.value, actual_value);
    }

    size_t size = std::min(tc.cycles.size(), bus->ops.size());
    for (size_t i = 0; i < size; ++i) {
        ASSERT_EQUAL(tc.cycles[i].address, bus->ops[i].addr);
        ASSERT_EQUAL(tc.cycles[i].value, bus->ops[i].val);
        ASSERT_EQUAL((tc.cycles[i].operation == "read"), bus->ops[i].read);
    }

    return true;
}

void cpu(ExecutionEnvironment &ee, NES::Test::MemoryBus *bus) {
    ee.disable_ppu = true;
    ee.run_single_step = true;
    ee.gui.mapper = nullptr;

    for (uint16_t i = 0x0; i <= 0xff; i++) {
        if (i == 0x93 || i == 0x9b || i == 0x9c || i == 0x9e || i == 0x9f
            || i == 0xbb || i == 0xcb) {
            std::cerr << "Disabled tests for 0x" << std::hex 
                      << (unsigned int)i 
                      << ". Unstable ops with weird behaviour" << std::endl;
            continue;
        }
        std::string filename = get_testspec_name(i);
        std::cout << "Executing " << filename << ". "; 

        std::ifstream file(filename);
        if (!file) {
            std::cerr << "Unable to open file. Aborting." << std::endl;
            return;
        }

        json j;
        file >> j;

        try {
            auto test_cases = j.get<std::vector<TestCase>>();

            for (const TestCase &tc : test_cases) {
                NES_LOG("cputest") << std::format("Running {}\n", tc.name);
                ee.power([&](NES::CPU &cpu, NES::PPU &ppu) {
                    cpu.PC = tc.initial.pc;
                    cpu.A = tc.initial.a;
                    cpu.X = tc.initial.x;
                    cpu.Y = tc.initial.y;
                    cpu.S = tc.initial.s;
                    cpu.P.status = tc.initial.p;
                    cpu.cycles = 0;
                });
                bus->mock_clear_ops();
                ee.pre_step_hook = [&](auto &ee) {
                    NES_LOG("CPU") << ee.logger.log() << std::endl;
                };
                for (const auto &ram : tc.initial.ram) {
                    bus->mock_write(ram.address, ram.value);
                }
                ee.run();
                if (!assert_final_state(ee, bus, tc)) {
                    i = 0x101;
                    break;
                }
            }
        } catch (json::exception &e) {
            std::cerr << "JSON parsing error: " << e.what() << "\n";
            return;
        }

        if (i != 0x101) std::cout << "Success" << std::endl;
    }
}

} // namespace Test

} // namespace NES
#endif
