#ifndef INC_2A03_TEST_CPU_H
#define INC_2A03_TEST_CPU_H

#include <json.hpp>
#include <iostream>
#include <fstream>
#include <format>

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

void cpu(ExecutionEnvironment &ee, NES::Test::MemoryBus *mock_bus) {
    std::string filename = get_testspec_name(0);
    
    std::cout << "Loading " << filename << std::endl; 
    
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
            std::cout << tc.name << "\n";
            std::cout << "Initial CPU: "
                      << "PC=0x" << std::hex << tc.initial.pc
                      << " S=0x" << std::hex << tc.initial.s
                      << " A=0x" << std::hex << tc.initial.a
                      << " X=0x" << std::hex << tc.initial.x
                      << " Y=0x" << std::hex << tc.initial.y
                      << " P=0x" << std::hex << tc.initial.p << "\n";

            std::cout << "Initial RAM:" << "\n";
            for (const auto &ram : tc.initial.ram) {
                std::cout << "Address: 0x" << ram.address
                          << ", Value: 0x" << ram.value << "\n";
            }

            std::cout << "Final CPU: "
                      << "PC=0x" << std::hex << tc.final.pc
                      << " S=0x" << std::hex << tc.final.s
                      << " A=0x" << std::hex << tc.final.a
                      << " X=0x" << std::hex << tc.final.x
                      << " Y=0x" << std::hex << tc.final.y
                      << " P=0x" << std::hex << tc.final.p << "\n";

            std::cout << "Final RAM:" << "\n";
            for (const auto &ram : tc.final.ram) {
                std::cout << "Address: 0x" << std::hex << ram.address
                          << ", Value: 0x" << std::hex << ram.value << "\n";
            }

            std::cout << "Memory accesses:" << "\n";
            for (const auto &cycle : tc.cycles) {
                std::cout << "Cycle Address: 0x" << std::hex << cycle.address
                          << ", Value: 0x" << std::hex << cycle.value
                          << ", Operation: " << cycle.operation << "\n";
            }
            break;
        }
    } catch (json::exception &e) {
        std::cerr << "JSON parsing error: " << e.what() << "\n";
        return;
    }
}

} // namespace Test

} // namespace NES
#endif
