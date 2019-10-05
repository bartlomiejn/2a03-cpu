#include <2a03/utils/string.h>

std::string str_to_hex(const std::string &input)
{
	static const char* const anum_table = "0123456789ABCDEF";
	size_t len = input.length();
	
	std::string output;
	output.reserve(2 * len);
	for (size_t i = 0; i < len; ++i)
	{
		const char c = input[i];
		output.push_back(anum_table[c >> 4]);
		output.push_back(anum_table[c & 15]);
	}
	return output;
}