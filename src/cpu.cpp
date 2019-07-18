#include <2a03/cpu.h>

/// Executes an instruction.
///
/// \param opcode Opcode of the instruction.
void NES::CPU::execute(uint8_t opcode)
{
	execute(opcode, 0x0);
}

/// Executes an instruction.
///
/// \param opcode Opcode of the instruction.
/// \param param Parameter of the instruction.
void NES::CPU::execute(uint8_t opcode, uint16_t param)
{
	switch (opcode)
	{
		// LDA (Load Accumulator with Memory)
		case 0xAD: // a
			A = mem[param];
			break;
		case 0xBD: // a,x
			A = mem[param + X];
			break;
		case 0xB9: // a,y
			A = mem[param + Y];
			break;
		case 0xA9: // #
			// TODO: Throw an error if higher than 0xFF?
			A = (uint8_t)(param & 0xFF);
			break;
		case 0xA5: // zp
			// TODO: Throw an error if higher than 0xFF?
			A = mem[param & 0xFF];
			break;
		case 0xA1: // (zp,x)
			
			break;
		case 0xB5: // zp,x
			break;
		case 0xB1: // (zp),y
			break;
	}
}