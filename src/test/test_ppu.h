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

    ee.logger.log_filename = gen_logname("palette_ram");

    std::cout << "Running " << palette_ram << std::endl;
    std::cout << "Saving logs to: " << ee.logger.log_filename.value()
              << std::endl;

    ee.load_iNESv1(palette_ram);
    ee.power(nullptr);
    ee.pre_step_hook = [](auto &ee) { ee.logger.log(); };
    ee.post_step_hook = [](auto &ee) {
        if (ee.cpu.PC == 0xE412) {  // Failure?
            std::cerr << "PC == E412. Terminating" << std::endl;
            ee.stop = true;
        }
    };

    std::cout << "Starting execution." << std::endl;
    ee.run();
    std::cout << "Finished execution. Saving log to file." << std::endl;
    ee.logger.save();
}

} // namespace Test

} // namespace NES

#endif
