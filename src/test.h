#include <ee.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <csignal>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

namespace NES {
enum TestState { running = 0x80, reset_required = 0x81 };

namespace Test {

void handle_sigint(int signum) {}

std::string ltrim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\n\r\f\v");
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string rtrim(const std::string &str) {
    size_t end = str.find_last_not_of(" \t\n\r\f\v");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string trim(std::string &line) {
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

std::string trim_ppu(std::string &line) {
    std::regex pat(R"(PPU:\s*\d+,\s*\d+)");
    return std::regex_replace(line, pat, "");
}

std::string gen_logname(std::string prefix) {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm lt = *std::localtime(&now_time);
    std::stringstream time_s;
    time_s << std::put_time(&lt, "%d_%H:%M:%S");
    return std::format("{}_{}.log", prefix, time_s.str());
}

void test_nestest_noppu_diff(std::string &logname) {
    std::cout << "Running test_nestest_noppu_diff" << std::endl;

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
        std::string noppu_log = trim_ppu(line_log);
        std::string noppu_nestest = trim_ppu(line_nestest);

        std::string trimmed_log = trim(noppu_log);
        std::string trimmed_nestest = trim(noppu_nestest);

        // Ignore PPU state for this test
        linebuf[idx] = noppu_log;
        linebuf_nestest[idx] = noppu_nestest;
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
    std::cout << "Setting up, PC=0xC000, cycles=7." << std::endl;

    ee.load_iNESv1(nestest_rom);
    ee.power([](NES::CPU &cpu) {
        cpu.PC = 0xC000;
        cpu.cycles = 7;
    });

    std::cout << "Entering runloop." << std::endl;

    char in = 0x0;
    unsigned int line = 0;
    std::string nestest_log = "nestest.log";
    std::string line_nestest;
    std::string line_ours;
    std::ifstream ifs(nestest_log);
    assert(ifs.is_open());

    ee.pre_step_hook = [&](auto &ee) {
        using namespace NES::Test;

        line_ours = ee.logger.log();
        line++;
        std::getline(ifs, line_nestest);

        std::string noppu_ours = trim_ppu(line_ours);
        std::string noppu_nestest = trim_ppu(line_nestest);
        std::string trimmed_ours = trim(noppu_ours);
        std::string trimmed_nestest = trim(noppu_nestest);

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
        if (ee.bus.read(0x02) != 0x0)  // Some sort of error occured:
        {
            std::cerr << "Nestest failure code: " << std::hex
                      << ee.bus.read(0x02) << "." << std::endl;
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

    NES::Test::test_nestest_noppu_diff(ee.logger.log_filename.value());
}

void ppu_tests(ExecutionEnvironment &ee) {
    using namespace NES::iNESv1;

    std::string palette_ram = "palette_ram.nes";
    ee.logger.log_filename = gen_logname("palette_ram");

    std::cout << "Running " << palette_ram << std::endl;
    std::cout << "Saving logs to: " << ee.logger.log_filename.value()
              << std::endl;

    ee.load_iNESv1(palette_ram);
    ee.power(nullptr);
    ee.pre_step_hook = [](auto &ee) { ee.logger.log(); };
    ee.post_step_hook = [](auto &ee) {
        if (ee.cpu.PC == 0xE412) {  // Failure
            std::cerr << "PC == E412. Terminating" << std::endl;
            ee.stop = true;
        }
    };

    std::cout << "Starting execution." << std::endl;
    ee.run();
    std::cout << "Finished execution. Saving log to file." << std::endl;
    ee.logger.save();
}

};  // namespace Test
}  // namespace NES
