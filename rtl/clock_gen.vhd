----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date:    12:58:05 02/08/2026 
-- Design Name: 
-- Module Name:    clock_gen - Behavioral 
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
library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;

entity clock_gen is
	port (
		reset: in std_logic;
		clk_in: in std_logic; 
	
		clk_div_20: out std_logic; -- div 20
		clk_div_60: out std_logic; -- div 60, a bit slower than PAL at 100MHz clk_in
	);
end clock_gen;

architecture RTL of clock_gen is
    signal counter_20 : integer range 0 to 19 := 0;
    signal counter_60 : integer range 0 to 59 := 0;
    signal clk_20 : STD_LOGIC := '0';
    signal clk_60 : STD_LOGIC := '0';
begin

    process(clk_in, reset)
    begin
        if reset = '1' then
            counter_20 <= 0;
            counter_60 <= 0;
            clk_20 <= '0';
            clk_60 <= '0';
        elsif rising_edge(clk_in) then
            -- Divide by 20
            if counter_20 = 19 then
                counter_20 <= 0;
                clk_20 <= not clk_20;
            else
                counter_20 <= counter_20 + 1;
            end if;
            
            -- Divide by 60
            if counter_60 = 59 then
                counter_60 <= 0;
                clk_60 <= not clk_60;
            else
                counter_60 <= counter_60 + 1;
            end if;
        end if;
    end process;
    
    clk_div_20 <= clk_20;
    clk_div_60 <= clk_60;
end rtl;


