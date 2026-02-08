----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    12:45:31 02/08/2026 
-- Design Name: 
-- Module Name:    ricoh-2a03 - RTL 
-- Project Name: 
-- Target Devices: 
-- Tool versions: 
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

--NES CPU (NTSC)
--RP2A03G / RP2A03H
--By Eli Dayan
--               _____   _____
--              |     \_/     |
--       SND1 - | 01       40 | - +5V (Vcc)
--       SND2 - | 02       39 | - STROBE (joypad)
--       /RST - | 03       38 | - EXT 44
--         A0 - | 04       37 | - EXT 45
--         A1 - | 05       36 | - /OE (joypad 0)
--         A2 - | 06       35 | - /OE (joypad 1)
--         A3 - | 07       34 | - R/W
--         A4 - | 08       33 | - /NMI
--         A5 - | 09       32 | - /IRQ
--         A6 - | 10       31 | - M2
--         A7 - | 11       30 | - GND*
--         A8 - | 12       29 | - CLK
--         A9 - | 13       28 | - D0
--        A10 - | 14       27 | - D1
--        A11 - | 15       26 | - D2
--        A12 - | 16       25 | - D3
--        A13 - | 17       24 | - D4
--        A14 - | 18       23 | - D5
--        A15 - | 19       22 | - D6
--        GND - | 20       21 | - D7
--              |_____________|
--
-- SND1 is the output for square waves 1 & 2.
-- SND2 is the output for triangle, noise, and DMC.
-- CLK is the NES master clock @ 21.47727 MHz
-- M2 is clocked at 1/2 the NTSC colorburst.  This signal is divided (by 12)
--  internally by the CPU and coincides with the CPU's clock speed of 
--  1.7897725 MHz.
-- All memory access occurs when M2 is high (/CE is low on a ROM)
-- EXT 44 and EXT 45 connect to the expansion connector pins 44 and 45 respectively.
-- STROBE (joypad) is connected to STA $4016 D0
-- /OE (joypads) is low during reads
-- GND* is tied low but may be the source of some signal

entity ricoh-2a03 is
	Port (
		MCLK: in std_logic;
		RST: in std_logic;
		NMI: in std_logic;
		IRQ: in std_logic;
		
		STROBE: out std_logic;
		A: out std_logic_vector(15 downto 0);
		D: out std_logic_vector(7 downto 0);
	);
end ricoh-2a03;

architecture RTL of ricoh-2a03 is


end RTL;

