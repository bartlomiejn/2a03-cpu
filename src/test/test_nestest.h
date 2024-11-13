#ifndef INC_2A03_TEST_NESTEST_H
#define INC_2A03_TEST_NESTEST_H

#include <cassert>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

namespace NES {

namespace Test {

extern std::string gen_logname(std::string prefix);

std::string ltrim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string rtrim(const std::string &str) {
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string trim(const std::string &line) {
    std::string result;
    bool in_whitespace = false;

    for (char ch : rtrim(ltrim(line))) {
        if (std::isspace(ch)) {
            if (!in_whitespace) {
                result += ' ';
                in_whitespace = true;
            }
        } else {
            result += ch;
            in_whitespace = false;
        }
    }

    return result;
}

/*
Don't have to trim ppu state anymore, keep it in case its ever needed
std::string trim_ppu(std::string &line) {
    std::regex pat(R"(PPU:\s*\d+,\s*\d+)");
    return std::regex_replace(line, pat, "");
}
*/
void nestest_diff_log(std::string &logname) {
    std::cout << "Running nestest_diff_log" << std::endl;

    std::string linebuf[5] = {""};
    std::string linebuf_nestest[5]{""};
    unsigned int idx = 0;
    unsigned int linenum = 1;
    std::string nestest_logname = "nestest.log";

    std::ifstream ifs_log(logname);
    std::ifstream ifs_nestest(nestest_logname);

    assert(ifs_log.is_open());
    assert(ifs_nestest.is_open());

    std::string line_log;
    std::string line_nestest;

    while (std::getline(ifs_nestest, line_nestest) &&
           std::getline(ifs_log, line_log)) {
        std::string trimmed_log = trim(line_log);
        std::string trimmed_nestest = trim(line_nestest);

        linebuf[idx] = line_log;
        linebuf_nestest[idx] = line_nestest;
        idx = (idx + 1) % 5;

        if (trimmed_log != trimmed_nestest) {
            unsigned int end = idx;
            std::cout << "Ours:" << std::endl;
            do {
                if (idx == (end - 1) % 5)
                    std::cout << "> ";
                else
                    std::cout << "  ";
                std::cout << linebuf[idx] << std::endl;
                idx = (idx + 1) % 5;
            } while (idx != end);

            std::cout << "nestest.log:" << std::endl;
            do {
                if (idx == (end - 1) % 5)
                    std::cout << "> ";
                else
                    std::cout << "  ";
                std::cout << linebuf_nestest[idx] << std::endl;
                idx = (idx + 1) % 5;
            } while (idx != end);
            std::cout << "DIFF AT LINE " << linenum << " FAILED" << std::endl;
            return;
        }

        linenum++;
    }
    std::cout << "Success" << std::endl;
}

void nestest(ExecutionEnvironment &ee) {
    using namespace NES::iNESv1;

    std::string nestest_rom = "nestest.nes";
    ee.logger.log_filename = gen_logname("nestest");

    std::cout << "Running " << nestest_rom << std::endl;
    std::cout << "Saving logs to: " << ee.logger.log_filename.value()
              << std::endl;
    std::cout << "Setting up, PC=0xC000, cycles=7, scan_x=21" << std::endl;

    ee.load_iNESv1(nestest_rom);
    ee.power([](NES::CPU &cpu, NES::PPU &ppu) {
        cpu.PC = 0xC000;
        cpu.cycles = 7;
        ppu.scan_x = 21;
    });

    std::cout << "Entering runloop." << std::endl;

    char in = 0x0;
    unsigned int line = 0;
    std::string nestest_log = "nestest.log";
    std::string line_nestest;
    std::string line_ours;
    std::ifstream ifs(nestest_log);
    assert(ifs.is_open());
    bool comp_check = false;

    ee.pre_step_hook = [&](auto &ee) {
        using namespace NES::Test;
        line_ours = ee.logger.log();
        if (!comp_check) {
            return;
        }
        line++;
        if (!std::getline(ifs, line_nestest)) {
            std::cerr << "Nestest.log ended. Check successful. "
                         "Continuing execution."
                      << std::endl;
            comp_check = false;
            return;
        }

        std::string trimmed_ours = trim(line_ours);
        std::string trimmed_nestest = trim(line_nestest);

        if (trimmed_ours != trimmed_nestest) {
            std::cerr << "Ours:    " << trimmed_ours << std::endl;
            std::cerr << "Nestest: " << trimmed_nestest << std::endl;
            std::cerr << "Line " << line << std::endl;
            std::cerr << "Continue with y, stop with n" << std::endl;
            in = 0x0;
            while (in != 'y' && in != 'n') {
                std::cin.get(in);
            }
            if (in == 'n') {
                ee.stop = true;
            }
        }
    };
    ee.post_step_hook = [&](auto &ee) {
        if (ee.bus->read(0x02) != 0x0)  // Some sort of error occured:
        {
            std::cerr << "Nestest failure code: " << std::hex
                      << ee.bus->read(0x02) << "." << std::endl;
            std::cerr << "Continue with y, stop with n" << std::endl;
            in = 0x0;
            while (in != 'y' && in != 'n') {
                std::cin.get(in);
            }
            if (in == 'n') {
                ee.stop = true;
            }
        }
    };

    ee.run();

    ifs.close();
    std::cout << "Finished execution." << std::endl;
    ee.logger.save();
    std::cout << "Saved log to: " << ee.logger.log_filename.value()
              << std::endl;

    NES::Test::nestest_diff_log(ee.logger.log_filename.value());
}

} // namespace Test

} // namespace NES
#endif
