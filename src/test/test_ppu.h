#ifndef INC_2A03_TEST_PPU_H
#define INC_2A03_TEST_PPU_H

#include <iostream>
#include <string>

namespace NES {

namespace Test {

extern std::string gen_logname(std::string prefix);

const std::string palette_ram = "palette_ram.nes";

void ppu(ExecutionEnvironment &ee) {
    using namespace NES::iNESv1;

    NES_LOG("pputest") << "Running " << palette_ram << std::endl;

    ee.load_iNESv1(palette_ram);
    ee.power(nullptr);
    ee.pre_step_hook = [](auto &ee) {
        NES_LOG("CPU") << ee.logger.log() << std::endl;
    };
    ee.post_step_hook = [](auto &ee) {
        if (ee.cpu.PC == 0xE412) {  // Failure?
            std::cerr << "PC == E412. Terminating" << std::endl;
            ee.stop = true;
        }
    };

    ee.run();
}

} // namespace Test

} // namespace NES

#endif
