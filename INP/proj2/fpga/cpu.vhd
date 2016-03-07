-- cpu.vhd: Simple 8-bit CPU (BrainFuck interpreter)
-- Copyright (C) 2015 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (1) / zapis (0)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is
    signal addr_sel : std_logic; -- 0 = code, 1 = data
    signal instr_ign : std_logic;

    -- Signals for program counter
    signal pc_reg : std_logic_vector(12 downto 0);      -- PC register
    signal pc_ld_val : std_logic_vector(12 downto 0);   -- PC value for jumps
    signal pc_ld : std_logic;                           -- Load value from pc_
    signal pc_inc : std_logic;                          -- Increment PC if set to '1'
    signal pc_dec : std_logic;
    signal pc_dir : std_logic := '0';                   -- Direction (0 = increment, 1 = decrement)

    -- Signals for data
    signal data_inc : std_logic;
    signal data_dec : std_logic;
    signal data_in : std_logic;

    -- Signals for data pointer
    signal data_ptr : std_logic_vector(15 downto 0);
    signal data_ptr_inc : std_logic;
    signal data_ptr_dec : std_logic;

    -- Temporary variable
    signal data_tmp : std_logic_vector(7 downto 0);
    signal data_tmp_ld : std_logic;

    -- Signals for FSM
    type fsm_state_t is (s_halt, s_fetch, s_fetch_wait, s_decode, s_execute, s_ptr_update, s_data_inc, s_data_dec,
                         s_while_begin, s_while_end, s_data_print, s_data_input, s_data_save, s_data_load,
                         s_prepare_next);
    signal curr_state : fsm_state_t;
    signal next_state : fsm_state_t;
begin

    -- Program counter (PC)
    program_counter: process(RESET, CLK, pc_ld, pc_inc)
    begin
        if(RESET = '1') then
            pc_reg <= (others => '0');
        elsif rising_edge(CLK) then
            if(pc_ld = '1') then
                pc_reg <= pc_ld_val;
            elsif(pc_inc = '1') then
                pc_reg <= pc_reg + 1;
            elsif(pc_dec = '1') then
                pc_reg <= pc_reg - 1;
            end if;
        end if;
    end process;

    data_register: process(CLK, DATA_RDATA, IN_DATA, data_inc, data_dec, data_in)
    begin
        if rising_edge(CLK) then
            if(data_inc = '1') then -- Increment data
                DATA_WDATA <= DATA_RDATA + 1;
            elsif(data_dec = '1') then -- Decrement data
                DATA_WDATA <= DATA_RDATA - 1;
            elsif(data_in = '1') then -- Save input data
                DATA_WDATA <= IN_DATA;
            elsif(data_tmp_ld = '1') then -- Load data from tmp variable
                DATA_WDATA <= data_tmp;
            end if;
        end if;
    end process;

    data_pointer: process(CLK, RESET, data_ptr, data_ptr_inc, data_ptr_dec)
    begin
        if(RESET = '1') then -- Reset data pointer to zero
            data_ptr <= X"1000";
        elsif rising_edge(CLK) then
            if(data_ptr_inc = '1') then -- Increment data pointer
                if(data_ptr = x"1FFF") then
                    data_ptr <= x"1000";
                else
                    data_ptr <= data_ptr + 1;
                end if;
            elsif(data_ptr_dec = '1') then -- Decrement data pointer
                if(data_ptr = x"1000") then
                    data_ptr <= x"1FFF";
                else
                    data_ptr <= data_ptr - 1;
                end if;
            end if;
        end if;
    end process;

    -- RAM mux
    DATA_ADDR <= data_ptr(12 downto 0) when (addr_sel = '0') else pc_reg;

    fsm_state: process(CLK, RESET, EN, next_state)
    begin
        if(RESET = '1') then
            curr_state <= s_fetch;
        elsif(rising_edge(CLK) and EN = '1') then
            curr_state <= next_state;
        end if;
    end process;

    fsm: process(CLK, DATA_RDATA, OUT_BUSY, IN_VLD, curr_state)
    begin
        DATA_EN <= '0';
        OUT_WE <= '0';
        IN_REQ <= '0';
        DATA_RDWR <= '0';
        pc_inc <= '0';
        pc_dec <= '0';
        pc_ld <= '0';
        data_inc <= '0';
        data_dec <= '0';
        data_in <= '0';
        data_ptr_inc <= '0';
        data_ptr_dec <= '0';
        data_tmp_ld <= '0';
        next_state <= s_halt;

        case curr_state is
            when s_fetch =>
                DATA_RDWR <= '1';
                DATA_EN <= '1';
                addr_sel <= '1';
                next_state <= s_fetch_wait;

            when s_fetch_wait =>
                next_state <= s_decode;

            when s_decode =>
                if(instr_ign = '1' and DATA_RDATA /= X"5B" and DATA_RDATA /= X"5D") then
                    next_state <= s_prepare_next;
                else
                case DATA_RDATA is
                    when X"3E" => -- > (increment pointer)
                        data_ptr_inc <= '1';
                        next_state <= s_ptr_update;

                    when X"3C" => -- < (decrement pointer)
                        data_ptr_dec <= '1';
                        next_state <= s_ptr_update;

                    when X"2B" => -- + (increment data)
                        addr_sel <= '0';
                        DATA_EN <= '1';
                        DATA_RDWR <= '1';
                        next_state <= s_data_inc;

                    when X"2D" => -- - (decrement data)
                        addr_sel <= '0';
                        DATA_EN <= '1';
                        DATA_RDWR <= '1';
                        next_state <= s_data_dec;

                    when x"5B" => -- [ (while loop - begin)
                        addr_sel <= '0';
                        DATA_EN <= '1';
                        DATA_RDWR <= '1';
                        next_state <= s_while_begin;

                    when x"5D" => -- ] (while loop - end)
                        addr_sel <= '0';
                        DATA_EN <= '1';
                        DATA_RDWR <= '1';
                        next_state <= s_while_end;        

                    when X"2E" => -- . (print value to display)
                        addr_sel <= '0';
                        DATA_EN <= '1';
                        DATA_RDWR <= '1';
                        next_state <= s_data_print;

                    when X"2C" => -- , (read value from keyboard)
                        addr_sel <= '0';
                        DATA_EN <= '1';
                        IN_REQ <= '1';
                        next_state <= s_data_input;

                    when X"24" => -- $ (save value to tmp variable)
                        addr_sel <= '0';
                        DATA_RDWR <= '1';
                        DATA_EN <= '1';
                        next_state <= s_data_save;

                    when X"21" => -- ! (load value from tmp variable)
                        addr_sel <= '0';
                        DATA_RDWR <= '1';
                        DATA_EN <= '1';
                        next_state <= s_data_load;

                    when X"00" => -- 0 (halt)
                        next_state <= s_halt;

                    when others =>
                        next_state <= s_prepare_next;
                end case;
                end if;

            when s_ptr_update =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                next_state <= s_prepare_next;

            when s_data_inc =>
                DATA_RDWR <= '1';
                DATA_EN <= '1';
                data_inc <= '1';
                next_state <= s_execute;

            when s_data_dec =>
                DATA_RDWR <= '1';
                DATA_EN <= '1';
                data_dec <= '1';
                next_state <= s_execute;

            when s_while_begin =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                if(DATA_RDATA = (DATA_RDATA'range => '0')) then
                    instr_ign <= '1';
                else
                    instr_ign <= '0';
                end if;
                pc_dir <= '0';
                next_state <= s_prepare_next;

            when s_while_end =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                if(DATA_RDATA /= (DATA_RDATA'range => '0')) then
                    pc_dir <= '1';
                    instr_ign <= '1';
                else
                    instr_ign <= '0';
                    pc_dir <= '0';
                end if;
                next_state <= s_prepare_next;

            when s_data_print =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                if(OUT_BUSY = '0') then
                    OUT_WE <= '1';
                    OUT_DATA <= DATA_RDATA;
                    next_state <= s_prepare_next;
                else
                    next_state <= s_data_print;
                end if;

            when s_data_input =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                if(IN_VLD = '1') then
                    data_in <= '1';
                    next_state <= s_execute;
                else
                    next_state <= s_data_input;
                end if;

            when s_data_save =>
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                data_tmp <= DATA_RDATA;
                next_state <= s_prepare_next;

            when s_data_load =>
                DATA_EN <= '1';
                DATA_RDWR <= '0';
                data_tmp_ld <= '1';
                next_state <= s_execute;

            when s_execute =>
                DATA_EN <= '1';
                next_state <= s_prepare_next;

            when s_prepare_next =>
                if(pc_dir = '0') then
                    pc_inc <= '1';
                else
                    pc_dec <= '1';
                end if;
                next_state <= s_fetch;

            when s_halt =>
                null;

            when others => null;
        end case;
    end process;
end behavioral;
