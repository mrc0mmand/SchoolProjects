-- Author: Frantisek Sumsal (xsumsa01)
-- Date: 18.11.2015
-- INP: Project 1

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
    port (
        SMCLK: in std_logic;
        RESET: in std_logic;
        ROW: out std_logic_vector(7 downto 0);
        LED: out std_logic_vector(7 downto 0)
    );
end entity ; -- ledc8x8

architecture behaviroal of ledc8x8 is
    signal cr_ce: std_logic := '0';
    signal cr_counter: std_logic_vector(21 downto 0);
    signal row_counter: std_logic_vector(7 downto 0);
    signal display_out: std_logic_vector(7 downto 0);
    signal led_mask: std_logic_vector(7 downto 0) := "00001111"; -- mask right half of the display
    signal display_timeout: std_logic_vector(21 downto 0) := (others => '0');
begin

    -- Generate SMCLK/256 enable signal
    row_signal: process(SMCLK, RESET)
    begin
        if RESET = '1' then
            cr_counter <= (others => '0');
        elsif rising_edge(SMCLK) then
            cr_counter <= cr_counter + 1;

            if(cr_counter(7 downto 0) = "11111111") then
                cr_ce <= '1';
            else
                cr_ce <= '0';
            end if;
        end if;
    end process;

    -- Rotate rows
    row_shifter: process(SMCLK, RESET, row_counter)
    begin
        if RESET = '1' then
            ROW <= "10000000";
            row_counter <= "10000000";
        elsif rising_edge(SMCLK) and cr_ce = '1' then
            case row_counter is
                when "10000000" => row_counter <= "01000000";
                when "01000000" => row_counter <= "00100000";
                when "00100000" => row_counter <= "00010000";
                when "00010000" => row_counter <= "00001000";
                when "00001000" => row_counter <= "00000100";
                when "00000100" => row_counter <= "00000010";
                when "00000010" => row_counter <= "00000001";
                when "00000001" => row_counter <= "10000000";
                when others => null;
            end case;
        end if;

        ROW <= row_counter;
    end process;

    -- Define a 8-bit sequence to 'display' for each line
    -- 0 = LED on, 1 = LED off
    display_decoder: process(SMCLK, display_out)
    begin
        if rising_edge(SMCLK) then
            case row_counter is
                when "10000000" => display_out <= ("00001111" or led_mask);
                when "01000000" => display_out <= ("01111001" or led_mask);
                when "00100000" => display_out <= ("00010110" or led_mask);
                when "00010000" => display_out <= ("01110111" or led_mask);
                when "00001000" => display_out <= ("01111001" or led_mask);
                when "00000100" => display_out <= ("11111110" or led_mask);
                when "00000010" => display_out <= ("11110110" or led_mask);
                when "00000001" => display_out <= ("11111001" or led_mask);
                when others => null;
            end case;
        end if;

        LED <= display_out;
    end process;

    -- Wait ~0.5 sec, then unmask second half of the display
    display_coordinator: process(SMCLK, display_out)
    begin
        if rising_edge(SMCLK) then
            -- 2 000 000 + 8 * 256 = ~0.5 sec delay
            if display_timeout = X"1E8C80" then
                led_mask <= (others => '0');
            else
                display_timeout <= display_timeout + 1;
            end if;
        end if;
    end process;
end architecture;
