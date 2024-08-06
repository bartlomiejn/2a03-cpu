#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <2a03/cpu.h>
#include <2a03/cart/load.h>
#include <2a03/cart/mapper.h>
#include <2a03/utils/logger.h>

#include <cassert>


namespace NES
{
	enum TestState
	{
		running = 0x80,
		reset_required = 0x81
	};

    namespace Test
    {
        std::string trim_whitespace(std::string &line)
        {
            std::string result;
            bool in_whitespace = false;

            for (char ch : line) {
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

        void diff_nestest(std::string &logname)
        {
            std::cout << "Running diff_nestest" << std::endl;

            std::string nestest_logname = "nestest.log";
            
            std::ifstream ifs_log(logname);
            std::ifstream ifs_nestest(nestest_logname);
           
            assert(ifs_log.is_open());
            assert(ifs_nestest.is_open());
 
            std::string line_log;
            std::string line_nestest;
 
            int linenum = 1;

            while (std::getline(ifs_nestest, line_nestest)
                   && std::getline(ifs_log, line_log)) {
    
                std::string trimmed_log = trim_whitespace(line_log);
                std::string trimmed_nestest = trim_whitespace(line_nestest);  

                std::cout << "Ours:        " << trimmed_log << std::endl;
                std::cout << "nestest.log: " << trimmed_nestest << std::endl;

                std::cout << "Ours:        " << line_log << std::endl;
                std::cout << "nestest.log: " << line_nestest << std::endl;

                if (trimmed_log != trimmed_nestest) {
                    std::cout << "DIFF AT LINE " << linenum << " FAILED" 
                              << std::endl;
                    return;
                }

                linenum++;
            }
        }
    };
}

std::string gen_timepid_logname() 
{
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm lt = *std::localtime(&now_time);
    pid_t pid = getpid();
    std::stringstream time_s;
    time_s << std::put_time(&lt, "%Y-%m-%d_%H:%M:%S");
    return std::format("{}_{}.log", time_s.str(), pid);
}

void run_nestest()
{
	using namespace NES::iNESv1;
	
    std::string logfile = gen_timepid_logname();

	NES::MemoryBus bus;
	NES::CPU cpu(bus);
	NES::CPULogger logger(cpu, bus);
    logger.instr_ostream = std::cerr;
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
	
	while (true)
	{
		logger.log();
		
		try { cpu.execute(); }
		catch (NES::InvalidOpcode err) { break; }
		
		if (bus.read(0x02) != 0x0) // Some sort of error occured:
		{
			std::cerr << "Nestest failure code: " << std::hex
				<< bus.read16(0x02) << "." << std::endl;
		}
	}
	
	std::cout << "Terminating." << std::endl;
	
	logger.save();

    std::cout << "Saved log to: " << logfile << std::endl;

    NES::Test::diff_nestest(logfile);
}

int main()
{
	run_nestest();
	return 0;
}
