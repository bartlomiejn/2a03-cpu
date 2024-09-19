#ifndef INC_2A03_INES_LOADER_H
#define INC_2A03_INES_LOADER_H

#include <cart/ines.h>

#include <string>

namespace NES {
namespace iNESv1 {
NES::iNESv1::Cartridge load(std::string &filename);

class InvalidFile {};
class InvalidMagicNumber {};
}  // namespace iNESv1
}  // namespace NES

#endif
