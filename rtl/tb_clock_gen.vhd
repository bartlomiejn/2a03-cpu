library ieee;
use ieee.std_logic_1164.all;
library std;
use std.textio.all;
 
entity tb_clkgen is
end tb_clkgen;
 
architecture Behavioral of tb_clkgen is 
    component clock_divider
        Port ( clk_in : in STD_LOGIC;
               reset : in STD_LOGIC;
               clk_div_20 : out STD_LOGIC;
               clk_div_60 : out STD_LOGIC);
    end component;
    
    signal clk_in : STD_LOGIC := '0';
    signal reset : STD_LOGIC := '0';
    signal clk_div_20 : STD_LOGIC;
    signal clk_div_60 : STD_LOGIC;

    constant clk_period : time := 10 ns;  -- 100 MHz clock
    
    signal sim_done : boolean := false;
	
begin
    uut: clock_divider
        Port map (
            clk_in => clk_in,
            reset => reset,
            clk_div_20 => clk_div_20,
            clk_div_60 => clk_div_60
        );

    stim_process: process
    begin
        -- Initial reset
        reset <= '1';
        wait for clk_period * 5;
        reset <= '0';
        
        -- Run for enough cycles to see multiple toggles
        -- For divide-by-60, we need at least 120 cycles to see a full period
        -- Let's run for 250 cycles to see multiple periods
        wait for clk_period * 240;
        
        -- Test reset during operation
        report "Testing reset during operation";
        reset <= '1';
        wait for clk_period * 3;
        reset <= '0';
        
        -- Run for more cycles after reset
        wait for clk_period * 160;
        
        -- End simulation
        report "Simulation completed successfully";
        sim_done <= true;
        wait;
    end process;
	
    monitor_process: process
        variable count_20 : integer := 0;
        variable count_60 : integer := 0;
        variable prev_clk_20 : STD_LOGIC := '0';
        variable prev_clk_60 : STD_LOGIC := '0';
    begin
        wait until reset = '0';
        wait until rising_edge(clk_in);
        
        while not sim_done loop
            wait until rising_edge(clk_in);
            
            -- Count input clocks between div_20 toggles
            if clk_div_20 /= prev_clk_20 then
                if count_20 /= 0 then
                    assert count_20 = 20
                        report "ERROR: clk_div_20 toggled after " & integer'image(count_20) & " cycles (expected 20)"
                        severity error;
                end if;
                count_20 := 0;
                prev_clk_20 := clk_div_20;
            end if;
            count_20 := count_20 + 1;
            
            -- Count input clocks between div_60 toggles
            if clk_div_60 /= prev_clk_60 then
                if count_60 /= 0 then
                    assert count_60 = 60
                        report "ERROR: clk_div_60 toggled after " & integer'image(count_60) & " cycles (expected 60)"
                        severity error;
                end if;
                count_60 := 0;
                prev_clk_60 := clk_div_60;
            end if;
            count_60 := count_60 + 1;
        end loop;
        
        wait;
    end process;

end;
