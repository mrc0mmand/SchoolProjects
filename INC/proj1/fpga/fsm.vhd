-- fsm.vhd: Finite State Machine
-- Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
-- Date: 15.4.2015
-- Keys:
--      1) 1278942770
--      2) 1406837048
library ieee;
use ieee.std_logic_1164.all;
-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity fsm is
port(
    CLK         : in  std_logic; -- Clock signal
    RESET       : in  std_logic; -- Reset signal

    -- Input signals
    -- Input KEY:
    -- Bits 0 - 9   => keys 0 - 9
    -- Bits 10 - 13 => keys A - D
    -- Bit 14       => Key *
    -- Bit 15       => Key #
    KEY         : in  std_logic_vector(15 downto 0); -- Pressed key
    CNT_OF      : in  std_logic; -- Overflow signal (after printing last character from buffer)

    -- Output signals
    FSM_CNT_CE  : out std_logic; -- Clock enable signal
    FSM_MX_MEM  : out std_logic; -- Memory segment multiplexor (Moore)
    FSM_MX_LCD  : out std_logic; -- LCD input multiplexor (Moore)
    FSM_LCD_WR  : out std_logic; -- Write to LCD
    FSM_LCD_CLR : out std_logic  -- Clear LCD
    );
end entity fsm;

-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of fsm is
    type t_state is (TEST1,     TEST2,      TEST3_KEY1, TEST3_KEY2, TEST4_KEY1,     TEST4_KEY2, 
                    TEST5_KEY1, TEST5_KEY2, TEST6_KEY1, TEST6_KEY2, TEST7_KEY1,     TEST7_KEY2,
                    TEST8_KEY1, TEST8_KEY2, TEST9_KEY1, TEST9_KEY2, TEST10_KEY1,    TEST10_KEY2,
                    TEST_SUCCESS, TEST_FAIL, PRINT_ACCESS_DENIED, PRINT_ACCESS_GRANTED, FINISH);
    signal present_state, next_state : t_state;

begin

-- Process for handling reset button
sync_logic : process(RESET, CLK)
begin
    if (RESET = '1') then
        present_state <= TEST1;
    elsif (CLK'event AND CLK = '1') then
        present_state <= next_state;
    end if;
end process sync_logic;

-- FSM
next_state_logic : process(present_state, KEY, CNT_OF)
begin
    case (present_state) is
    -- Test first number of each key
    when TEST1 =>
        next_state <= TEST1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(1) = '1') then -- Both keys have '1' as they first number
            next_state <= TEST2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test second number of each key
    when TEST2 =>
        next_state <= TEST2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(2) = '1') then -- Second number of the first key is '2'
            next_state <= TEST3_KEY1;
        elsif (KEY(4) = '1') then -- Second number of the second key is '4'
            next_state <= TEST3_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test third number of the first key
    when TEST3_KEY1 =>
        next_state <= TEST3_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(7) = '1') then -- Third number of the first key is '7'
            next_state <= TEST4_KEY1;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test third number of the second key
    when TEST3_KEY2 =>
        next_state <= TEST3_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(0) = '1') then -- Third number of the second key is '0'
            next_state <= TEST4_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test fourth number of the first key
    when TEST4_KEY1 =>
        next_state <= TEST4_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(8) = '1') then -- Fourth number of the first key is '8'
            next_state <= TEST5_KEY1;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test fourth number of the second key
    when TEST4_KEY2 =>
        next_state <= TEST4_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(6) = '1') then -- Fourth number of the second key is '6'
            next_state <= TEST5_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test fifth number of the first key
    when TEST5_KEY1 =>
        next_state <= TEST5_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(9) = '1') then -- Fifth number of the first key is '9'
            next_state <= TEST6_KEY1;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test fifth number of the second key
    when TEST5_KEY2 =>
        next_state <= TEST5_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(8) = '1') then -- Fifth number of the second key is '8'
            next_state <= TEST6_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;


    -- Test sixth number of the first key
    when TEST6_KEY1 =>
        next_state <= TEST6_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(4) = '1') then -- Sixth number of the first key is '4'
            next_state <= TEST7_KEY1;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test sixth number of the second key
    when TEST6_KEY2 =>
        next_state <= TEST6_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(3) = '1') then -- Sixth number of the second key is '3'
            next_state <= TEST7_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test seventh number of the first key
    when TEST7_KEY1 =>
        next_state <= TEST7_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(2) = '1') then -- Seventh number of the first key is '2'
            next_state <= TEST8_KEY1;   
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL; 
        end if;

    -- Test seventh number of the second key
    when TEST7_KEY2 =>
        next_state <= TEST7_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(7) = '1') then -- Seventh number of the second key is '7'
            next_state <= TEST8_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test eight number of the first key
    when TEST8_KEY1 =>
        next_state <= TEST8_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(7) = '1') then -- Eight number of the first key is '7'
            next_state <= TEST9_KEY1; 
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;   
        end if;

    -- Test eight number of the second key
    when TEST8_KEY2 =>
        next_state <= TEST8_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(0) = '1') then -- Eight number of the second key is '0'
            next_state <= TEST9_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test ninth number of the first key
    when TEST9_KEY1 =>
        next_state <= TEST9_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(7) = '1') then -- Ninth number of the first key is '7'
            next_state <= TEST10_KEY1; 
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;  
        end if;

    -- Test ninth number of the second key
    when TEST9_KEY2 =>
        next_state <= TEST9_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(4) = '1') then -- Ninth number of the second key is '4'
            next_state <= TEST10_KEY2;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- Test tenth (and last) number of the first key
    when TEST10_KEY1 =>
        next_state <= TEST10_KEY1;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(0) = '1') then -- Tenth number of the first key is '0'
            next_state <= TEST_SUCCESS;   
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL; 
        end if;

    -- Test tenth (and last) number of the second key
    when TEST10_KEY2 =>
        next_state <= TEST10_KEY2;
        if (KEY(15) = '1') then -- Confirm key ('#' is pressed)
            next_state <= PRINT_ACCESS_DENIED; -- We haven't checked whole keys => invalid key is entered
        elsif (KEY(8) = '1') then -- tenth number of the second key is '8'
            next_state <= TEST_SUCCESS;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got invalid input
            next_state <= TEST_FAIL;
        end if;

    -- We got correct sequence of numbers, let's wait for confirmation (pressing '#' key)
    when TEST_SUCCESS =>
        next_state <= TEST_SUCCESS;
        if (KEY(15) = '1') then
            next_state <= PRINT_ACCESS_GRANTED;
        elsif (KEY(15 downto 0) /= "0000000000000000") then -- We got another number, which invalidates current sequence
            next_state <= TEST_FAIL;
        end if;

    -- We got incorrect sequence of numbers, let's wait for confirmation (pressing '#' key)
    when TEST_FAIL =>
        next_state <= TEST_FAIL;
        if (KEY(15) = '1') then
            next_state <= PRINT_ACCESS_DENIED;
        end if;

    -- Print 'Pristup odepren' to LCD
    when PRINT_ACCESS_DENIED =>
        next_state <= PRINT_ACCESS_DENIED;
        if (CNT_OF = '1') then
            next_state <= FINISH;
        end if;

    -- Print 'Pristup povolen' to LCD
    when PRINT_ACCESS_GRANTED =>
        next_state <= PRINT_ACCESS_GRANTED;
        if (CNT_OF = '1') then
            next_state <= FINISH;
        end if;

    -- If '#' key is pressed, proceed back to 'login' screen
    when FINISH =>
        next_state <= FINISH;
        if (KEY(15) = '1') then
            next_state <= TEST1; 
        end if;
    
    -- Else return to first check
    when others =>
        next_state <= TEST1;
    end case;
end process next_state_logic;

-- Output handling
output_logic : process(present_state, KEY)
begin
    FSM_CNT_CE     <= '0'; -- Clock enable signal (0 => do nothing, 1 => print characters sequentially to LCD)
    FSM_MX_MEM     <= '0'; -- Memory segment multiplexor (0 => mem. seg. 'Pristup odepren', 1 => 'Pristup povolen')
    FSM_MX_LCD     <= '0'; -- LCD input multiplexor (0 => print '*', 1 => print message from ROM)
    FSM_LCD_WR     <= '0'; -- Write character to LCD (0 => do nothing, 1 => write output to LCD)
    FSM_LCD_CLR    <= '0'; -- Clear LCD (0 => do nothing, 1 => clear LCD)

    case (present_state) is
    -- Print 'Pristup odepren' to LCD 
    when PRINT_ACCESS_DENIED =>
        FSM_CNT_CE     <= '1'; -- Clock enable signal
        FSM_MX_LCD     <= '1'; -- LCD input multiplexor
        FSM_MX_MEM     <= '0'; -- Memory segment with message
        FSM_LCD_WR     <= '1'; -- Write to LCD

    -- Print 'Pristup povolen'
    when PRINT_ACCESS_GRANTED =>
        FSM_CNT_CE     <= '1'; -- Clock enable signal
        FSM_MX_LCD     <= '1'; -- LCD input multiplexor
        FSM_MX_MEM     <= '1'; -- Memory segment with message
        FSM_LCD_WR     <= '1'; -- Write to LCD

    -- If '#' is pressed, clear the LCD
    when FINISH =>
        if (KEY(15) = '1') then
            FSM_LCD_CLR    <= '1';
        end if;

    when others =>
        -- Print '*' to LCD display when (almost) any key is pressed
        if (KEY(14 downto 0) /= "000000000000000") then
            FSM_LCD_WR     <= '1';
        end if;
        -- Clear LCD display when '#' key is pressed
        if (KEY(15) = '1') then
            FSM_LCD_CLR    <= '1';
        end if;
    end case;
end process output_logic;

end architecture behavioral;
