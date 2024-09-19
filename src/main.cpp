#include <2a03/cart/load.h>
#include <2a03/cart/mapper.h>
#include <2a03/cpu.h>
#include <2a03/utils/logger.h>
#include <unistd.h>

#include <cassert>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

namespace NES {
enum TestState { running = 0x80, reset_required = 0x81 };

namespace Test {
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

void diff_nestest_noppu(std::string &logname) {
    std::cout << "Running diff_nestest_noppu" << std::endl;

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
}
};  // namespace Test
}  // namespace NES

std::string gen_timepid_logname() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm lt = *std::localtime(&now_time);
    pid_t pid = getpid();
    std::stringstream time_s;
    time_s << std::put_time(&lt, "%Y-%m-%d_%H:%M:%S");
    return std::format("{}_{}.log", time_s.str(), pid);
}

void run_nestest(bool log_to_cerr) {
    using namespace NES::iNESv1;

    std::string logfile = gen_timepid_logname();

    NES::MemoryBus bus;
    NES::CPU cpu(bus);
    NES::CPULogger logger(cpu, bus);
    if (log_to_cerr) logger.instr_ostream = std::cerr;
    logger.log_filename = logfile;

    std::string test_file = "nestest.nes";

    std::cout << "Loading " << test_file << "." << std::endl;

    Cartridge cartridge = load(test_file);
    NES::iNESv1::Mapper::Base *mapper = Mapper::mapper(cartridge);
    bus.mapper = mapper;

    std::cout << "Powering up." << std::endl;

    cpu.power();

    cpu.PC = 0xC000;
    cpu.cycles = 7;

    std::cout << "Entering runloop." << std::endl;

    char in = 0x0;
    unsigned int line = 0;
    std::string nestest_log = "nestest.log";
    std::string line_nestest;
    std::string line_ours;
    std::ifstream ifs(nestest_log);
    assert(ifs.is_open());

    while (true) {
        using namespace NES::Test;

        line++;
        std::getline(ifs, line_nestest);
        line_ours = logger.log();

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
                break;
            }
        }

        try {
            cpu.execute();
        } catch (NES::InvalidOpcode err) {
            break;
        }

        if (bus.read(0x02) != 0x0)  // Some sort of error occured:
        {
            std::cerr << "Nestest failure code: " << std::hex << bus.read(0x02)
                      << "." << std::endl;
            std::cerr << "Continue with y, stop with n" << std::endl;
            in = 0x0;
            while (in != 'y' && in != 'n') {
                std::cin.get(in);
            }
            if (in == 'n') {
                break;
            }
        }
    }

    ifs.close();

    std::cout << "Terminating." << std::endl;

    logger.save();

    std::cout << "Saved log to: " << logfile << std::endl;

    // Run nestest.log diff test without PPU state
    NES::Test::diff_nestest_noppu(logfile);

    std::cout << "Finished execution." << std::endl;
}

int main(int argc, char *argv[]) {
    int opt;
    bool log_to_cerr = false;

    while ((opt = getopt(argc, argv, "c")) != -1) {
        switch (opt) {
            case 'c':
                log_to_cerr = true;
                break;
            default:
                std::cerr << "Usage: " << argv[0] << " [-c]" << std::endl;
                return -1;
        }
    }
    run_nestest(log_to_cerr);
    return 0;
}
