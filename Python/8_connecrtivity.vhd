----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 2022/11/29 18:40:18
-- Design Name: 
-- Module Name: hysteresis - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
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
use IEEE.numeric_std.all;
use work.pkg.all;
use work.const_def.all;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity eight_connectivity is
    generic(    
        data_bits : integer := 8;
        ksize : integer := 3;
        label_bits : integer := 10
    );
    port (
        i_clk      : in  std_logic;
        i_rst      : in  std_logic;
        i_enable   : in  std_logic;
        i_gradient : in std_logic_vector (data_bits-1 downto 0);
        o_ccl : out std_logic_vector(label_bits - 1 downto 0);
        o_debug : out std_logic_vector(label_bits - 1 downto 0);
        o_debug_1: out std_logic_vector(label_bits - 1 downto 0)
    );
end eight_connectivity;

architecture Behavioral of eight_connectivity is

component matrix_nxn is
	generic(
		ksize  : integer;
		data_bits : integer
	);
	port (
        i_clk    :    in STD_LOGIC;
        i_rst    :    in STD_LOGIC;
        i_enable :    in STD_LOGIC;     
        i_data   :    in STD_LOGIC_VECTOR(data_bits-1 downto 0);
        o_data   : inout out_matrix
	);
end component;
component FIFO_RTL is
    generic (
        g_WIDTH : integer;
        g_DEPTH : integer
    );
    port (
        i_rst   :  in STD_LOGIC;
        i_clk   :  in STD_LOGIC;   
        i_wr_en :  in STD_LOGIC;    
        i_rd_en :  in STD_LOGIC;
        i_data  :  in STD_LOGIC_VECTOR(g_WIDTH-1 downto 0);
        o_data  : out STD_LOGIC_VECTOR(g_WIDTH-1 downto 0);
        o_full  : out STD_LOGIC;    
        o_empty : out STD_LOGIC
    );
end component;
component blk_mem_gen_1 is 
    port (
        clka : IN STD_LOGIC;
        ena : IN STD_LOGIC;
        wea : IN STD_LOGIC_VECTOR(0 DOWNTO 0);
        addra : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
        dina : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
        clkb : IN STD_LOGIC;
        enb : IN STD_LOGIC;
        addrb : IN STD_LOGIC_VECTOR(9 DOWNTO 0);
        doutb : OUT STD_LOGIC_VECTOR(9 DOWNTO 0)
    ); 
end component;


type row_data is array(2 downto 0) of std_logic_vector(label_bits-1 downto 0);
type delay_reg is array(1 downto 0) of std_logic_vector(label_bits-1 downto 0);

type EQUIVALENCE_WRITE is (IDLE, WRITE_DATA_A, WRITE_DATA_B);
type EQUIVALENCE_BRAM is (IDLE, FIRST_SCAN, SECOND_SCAN);
type BRAM_REFRESH is (IDLE,READ_BRAM, WRITE_BRAM);
signal FSM_E_WRITE : EQUIVALENCE_WRITE;
signal FSM_E_BRAM : EQUIVALENCE_BRAM;
signal DATA_A_ACESS,DATA_B_ACESS : BRAM_REFRESH;
signal label_element_reg : delay_reg;

signal matrix   :out_matrix(0 to ksize-1 , 0 to ksize-1);
signal pre_edge_track ,local_min_label,local_max_label: std_logic_vector(label_bits-1 downto 0);
signal pre_row_edge_track : row_data;
signal rd_cnt,wr_cnt : integer range 0 to img_width;
signal rd_en ,wr_en: std_logic;
signal label_index : integer;
signal label_element : std_logic_vector(label_bits-1 downto 0);


signal fifo_in,equivalence : std_logic_vector(label_bits-1 downto 0);
signal min_value_a,min_value_b : STD_LOGIC_VECTOR(label_bits-1 downto 0);
signal max_key_a,max_key_b : std_logic;

signal bram_wr : std_logic_vector(0 downto 0);
signal bram_rd,gg : std_logic;
signal label_a,local_min_label_a,global_min_a,ccl_reg : std_logic_vector(label_bits-1 downto 0);
signal cnt : integer;
signal bram_dina,bram_addra,bram_doutb,bram_addrb : STD_LOGIC_VECTOR(label_bits-1 downto 0);

signal first_scan_done,second_scan_done : std_logic;
signal DATA_A_READ_DONE,DATA_A_WRITE_DONE,DATA_B_READ_DONE,DATA_B_WRITE_DONE : std_logic;

signal first_scan_cnt : integer;
signal SECOND_SCAN_START,SECOND,ccl_fifo_wr : std_logic;
constant first_scan_delay :integer := img_width * 2 + 8;
begin

matrix_1 :matrix_nxn  
    generic map ( 
        ksize => ksize,
        data_bits => data_bits         
    )  
    port map(
        i_clk => i_clk,
        i_rst => i_rst,
        i_enable => '1',
        i_data => i_gradient,
        o_data => matrix
    );
--@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ stage 1(double threshold)
new_label : process(i_rst , i_enable , matrix)
begin
    if i_rst = '1' then
        label_index <= 0;
    elsif i_enable = '1' then 
        if to_integer(unsigned(matrix(1,1))) > 0 then
            if to_integer(unsigned(matrix(0,0))) > 0 or 
               to_integer(unsigned(matrix(0,1))) > 0 or 
               to_integer(unsigned(matrix(0,2))) > 0 or 
               to_integer(unsigned(matrix(1,0))) > 0 then
            else
                label_index <= label_index + 1;
            end if;
        end if;
    end if;    
end process;
first_scan_process : process(i_rst,i_clk,matrix,i_enable,label_index)--labeled all points
begin
    if i_rst = '1' then
        label_element <= (others => '0');
    elsif rising_edge(i_clk) then
        if i_enable = '1' then
            if to_integer(unsigned(matrix(1,1))) > 0 then
                label_element <= std_logic_vector(to_unsigned(label_index,label_bits));
            else
                label_element <= (others => '0');
            end if;
        end if;
    end if;
end process;
--@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ stage 2,3,4(delay)
row_edge_track : process(i_rst , i_clk,pre_edge_track,pre_row_edge_track)
begin
    if i_rst = '1' then
        pre_row_edge_track <= (others => (others =>'0'));
    elsif rising_edge(i_clk)then
        pre_row_edge_track(0) <= pre_edge_track;
        for i in 1 to ksize-1 loop
            pre_row_edge_track(i) <= pre_row_edge_track(i-1);    
        end loop;
    end if;
end process;
delay : process(i_rst,i_clk,label_element,label_element_reg)
begin
    if i_rst = '1' then
        label_element_reg <= (others => (others => '0'));
    elsif rising_edge(i_clk)then
        label_element_reg(0) <= label_element;
        for i in 1 to 1 loop
            label_element_reg(i) <= label_element_reg(i-1);
        end loop;  
    end if;
end process;
local_min_label_process: process(i_rst, i_clk,label_element_reg,local_min_label,i_enable,pre_row_edge_track,min_value_a,min_value_b)
begin
    if i_rst = '1' then
        min_value_a <= (others =>'0');
        min_value_b <= (others =>'0');
        max_key_a <= '0';  
        max_key_b <= '0';  
    elsif i_enable = '1' then
        case to_integer(unsigned(label_element_reg(1))) is
            when 0 =>              
                min_value_a <= (others =>'0');
                min_value_b <= (others =>'0');                
            when others =>
                if to_integer(unsigned(pre_row_edge_track(0))) > 0 or to_integer(unsigned(pre_row_edge_track(2))) > 0 then
                    max_key_a <= '1';  
                    if to_integer(unsigned(pre_row_edge_track(0))) > 0 and to_integer(unsigned(pre_row_edge_track(2))) > 0 then
                        if pre_row_edge_track(0) > pre_row_edge_track(2) then
                            min_value_a <= pre_row_edge_track(2);
                        else
                            min_value_a <= pre_row_edge_track(0);
                        end if;
                    else
                        if pre_row_edge_track(0) > pre_row_edge_track(2) then
                            min_value_a <= pre_row_edge_track(0);
                        else
                            min_value_a <= pre_row_edge_track(2);
                        end if;
                    end if;
                else
                    max_key_a <= '0';                  
                    min_value_a <= label_element_reg(1);
                end if;                        
                if to_integer(unsigned(local_min_label)) > 0 or to_integer(unsigned(pre_row_edge_track(1))) > 0 then
                    max_key_b <= '1';  
                    if to_integer(unsigned(local_min_label)) > 0 and to_integer(unsigned(pre_row_edge_track(1))) > 0 then
                        if pre_row_edge_track(1) > local_min_label then
                            min_value_b <= local_min_label;
                        else
                            min_value_b <= pre_row_edge_track(1);
                        end if;
                    else
                        if pre_row_edge_track(1) > local_min_label then
                            min_value_b <= pre_row_edge_track(1);
                        else
                            min_value_b <= local_min_label;
                        end if;
                    end if;
                else
                    max_key_b <= '0';
                    min_value_b <= label_element_reg(1);
                end if;
        end case;
    end if;
end process;
c: process(i_rst, i_clk,label_element_reg,i_enable,min_value_a,min_value_b,max_key_b,max_key_a)
begin
    if i_rst = '1' then
        local_min_label <= (others =>'0');
        local_max_label <= (others =>'0');
        o_debug <= (others => '0');
        gg <= '0';
    elsif rising_edge(i_clk) then
        if i_enable = '1' then
            case to_integer(unsigned(label_element_reg(1))) is
                when 0 =>
                    gg <= '0';           
                    local_min_label <= (others => '0');
                    local_max_label <= (others => '0');
                    o_debug <= (others => '0');
                when others =>
                    gg <= '1';
                    if min_value_a > min_value_b then                        
                        local_min_label <= min_value_b;
                        if max_key_a = '1' then
                            local_max_label <= min_value_a;
                        else
                            local_max_label <= min_value_b;
                        end if;
                        o_debug <= min_value_b;
                    else
                        local_min_label <= min_value_a;
                        if max_key_b = '1' then
                            local_max_label <= min_value_b;
                        else
                            local_max_label <= min_value_a;
                        end if;
                        o_debug <= min_value_a;
                    end if;                    
            end case;
        end if;
    end if;
end process;
rd_enable_process : process(i_rst ,i_clk,i_enable,rd_cnt)
begin
    if i_rst = '1'then
        rd_cnt <= 0;
        rd_en <= '0';
    elsif rising_edge(i_clk)then
        if i_enable = '1' then
            if rd_cnt < img_width then
                rd_cnt <= rd_cnt + 1;
                rd_en <= '0';
            else
                rd_en <= '1';            
            end if;
        else
            rd_en <= '0';
        end if;
    end if;
end process;
wr_enable_process : process(i_rst ,i_clk,i_enable,wr_cnt)
begin
    if i_rst = '1'then
        wr_cnt <= 0;
        wr_en <= '0';
    elsif rising_edge(i_clk)then
        if i_enable = '1' then
            if wr_cnt < 3 then
                wr_cnt <= wr_cnt + 1;
                wr_en <= '0';
            else
                wr_en <= '1';            
            end if;
        else
            wr_en <= '0';
        end if;
    end if;
end process;
label_fifo: FIFO_RTL
    generic map(
        g_WIDTH  => label_bits,
        g_DEPTH  => img_width+100
    )
    port map(
        i_clk     =>  i_clk,
        i_rst     =>  i_rst,                
        i_wr_en   =>  wr_en,
        i_rd_en   =>  rd_en,
        i_data =>  local_min_label,
        o_data =>  pre_edge_track
    );   
SYS_FSM : process(i_rst,FSM_E_BRAM,i_enable,first_scan_done,second_scan_done)
begin
    if i_rst = '1' then
        FSM_E_BRAM <= IDLE;
    else
        case FSM_E_BRAM is
            when IDLE => 
                if i_enable = '1' then
                    FSM_E_BRAM <= FIRST_SCAN;
                else
                    FSM_E_BRAM <= IDLE;
                end if;
            when FIRST_SCAN =>
                if first_scan_done = '1' then
                    FSM_E_BRAM <= SECOND_SCAN;
                else
                    FSM_E_BRAM <= FIRST_SCAN;
                end if;
            when SECOND_SCAN =>
                if second_scan_done = '1' then
                    FSM_E_BRAM <= IDLE;
                else
                    FSM_E_BRAM <= SECOND_SCAN;
                end if;            
            when others => NULL;
        end case;
    end if;
end process;
SYS_act : process(i_rst,i_clk,FSM_E_BRAM,first_scan_cnt)
begin
    if i_rst = '1' then
        first_scan_done <= '0';
        second_scan_done <= '0';
        first_scan_cnt <= 0;
        SECOND_SCAN_START <= '0';
        SECOND <= '0';
    elsif rising_edge(i_clk)then
        case FSM_E_BRAM is
            when FIRST_SCAN =>
                SECOND <= '0';
                second_scan_done <= '0';
                if first_scan_cnt > img_width*2+5 then
                    SECOND_SCAN_START <= '1';
                else
                    SECOND_SCAN_START <= '0';
                end if;
                if first_scan_cnt < img_width*img_height+first_scan_delay then
                    first_scan_cnt <= first_scan_cnt + 1;
                    first_scan_done <= '0';
                else
                    first_scan_cnt <= 0;
                    first_scan_done <= '1';
                end if;
            when SECOND_SCAN =>
                SECOND <= '1';
                first_scan_done <= '0';                
                if first_scan_cnt < img_width*img_height+first_scan_delay-8 then
                    first_scan_cnt <= first_scan_cnt + 1;
                    second_scan_done <= '0';
                else
                    first_scan_cnt <= 0;
                    second_scan_done <= '1';
                end if;           
            when others =>
                first_scan_done <= '0';
                second_scan_done <= '0';
                SECOND <= '0';
        end case;
    end if;
end process;
equivalence_bram_process : process(i_clk,i_rst,FSM_E_BRAM,FSM_E_WRITE,gg)
begin
    if i_rst = '1' then        
        FSM_E_WRITE <= IDLE;
    else
        case FSM_E_BRAM is 
            when FIRST_SCAN =>
                case FSM_E_WRITE is
                    when IDLE =>
                        if gg = '1' then
                            FSM_E_WRITE <= WRITE_DATA_A;            
                        else
                            FSM_E_WRITE <= IDLE;
                        end if;
                    when WRITE_DATA_A =>
                        if gg = '1' then
                            FSM_E_WRITE <= WRITE_DATA_A;                    
                        else
                            FSM_E_WRITE <= WRITE_DATA_B;                                   
                        end if;
                    when WRITE_DATA_B =>
                        if gg = '1' then                    
                            FSM_E_WRITE <= WRITE_DATA_A;
                        else
                            FSM_E_WRITE <= WRITE_DATA_B;                      
                        end if;                                
                    when others => NULL;
               end case;
               when others => NULL;
        end case;
    end if;
end process;
DATA_ACESS_process : process(i_rst,i_clk,FSM_E_BRAM,FSM_E_WRITE,DATA_A_ACESS,DATA_B_ACESS,DATA_A_READ_DONE,DATA_B_READ_DONE,DATA_A_WRITE_DONE,DATA_B_WRITE_DONE)
begin
    if i_rst = '1' then 
        DATA_A_ACESS <= READ_BRAM;
        DATA_B_ACESS <= READ_BRAM;
    elsif rising_edge(i_clk)then
        case FSM_E_BRAM is 
            when FIRST_SCAN =>
                case FSM_E_WRITE is
                    when WRITE_DATA_A =>
                        DATA_B_ACESS <= READ_BRAM;
                        case DATA_A_ACESS is
                            when IDLE => DATA_A_ACESS <= READ_BRAM;
                            when READ_BRAM =>
                                if DATA_A_READ_DONE = '1' then
                                    DATA_A_ACESS <= WRITE_BRAM;
                                else
                                    DATA_A_ACESS <= READ_BRAM;
                                end if;
                            when WRITE_BRAM =>
                                if DATA_A_WRITE_DONE = '1' then
                                    DATA_A_ACESS <= READ_BRAM;
                                else
                                    DATA_A_ACESS <= WRITE_BRAM;
                                end if;            
                            when others => NULL;
                        end case;
                    when WRITE_DATA_B =>
                        DATA_A_ACESS <= READ_BRAM;
                        case DATA_B_ACESS is
                            when IDLE => DATA_B_ACESS <= READ_BRAM;
                            when READ_BRAM =>
                                if DATA_B_READ_DONE = '1' then
                                    DATA_B_ACESS <= WRITE_BRAM;
                                else
                                    DATA_B_ACESS <= READ_BRAM;
                                end if;
                            when WRITE_BRAM =>
                                if DATA_B_WRITE_DONE = '1' then
                                    DATA_B_ACESS <= READ_BRAM;
                                else
                                    DATA_B_ACESS <= WRITE_BRAM;
                                end if;            
                            when others => NULL;
                        end case;
                    when others => NULL;
                end case;
            when others =>
                DATA_A_ACESS <= READ_BRAM;
                DATA_B_ACESS <= READ_BRAM;
        end case;
    end if;
end process;
DATA_ACESS_ACT : process(i_rst,i_clk,FSM_E_BRAM,DATA_A_ACESS,DATA_B_ACESS,bram_dina,bram_doutb,local_min_label_a)
begin
    if i_rst = '1' then 
        label_a <= (others => '0');
        local_min_label_a <= (others => '0');
        bram_wr <= "0";
        bram_rd <= '0';
        bram_addrb <= (others => '0');
        bram_addra <= (others => '0');
        bram_dina <= (others => '0');
        DATA_A_READ_DONE <= '0';
        DATA_A_WRITE_DONE <= '0';
        ccl_fifo_wr <= '0';
        ccl_reg <= (others => '0');
    else
        case FSM_E_BRAM is 
            when FIRST_SCAN =>
                case FSM_E_WRITE is
                    when WRITE_DATA_A =>    
                        ccl_fifo_wr <= '0'; 
                        case DATA_A_ACESS is                            
                            when READ_BRAM =>
                                DATA_A_WRITE_DONE <= '0';
                                DATA_A_READ_DONE  <= '1';
                                bram_wr <= "0";
                                bram_rd <= '1';
                                label_a <= local_max_label;
                                local_min_label_a <= local_min_label;
                                bram_addrb <= local_min_label;
                            when WRITE_BRAM =>
                                DATA_A_WRITE_DONE <= '1';
                                DATA_A_READ_DONE  <= '0';            
                                bram_wr <= "1";
                                bram_addra <= label_a;
                                if to_integer(unsigned(label_a)) = 0 then
                                    bram_dina <= (others => '0');
                                else                                
                                    if to_integer(unsigned(bram_doutb)) > 0 then
                                        bram_dina <= bram_doutb;
                                    else
                                        bram_dina <= local_min_label;
                                    end if;
                                end if;
                            when others =>
                                bram_wr <= "0";
                                bram_rd <= '0';
                                bram_addrb <= (others => '0');
                                bram_addra <= (others => '0');
                                bram_dina <= (others => '0');                                
                        end case;
                    when WRITE_DATA_B =>
                        bram_wr <= "0";
                        bram_addra <= (others => '0');
                        bram_dina <= (others => '0');
                        case DATA_B_ACESS is
                            when READ_BRAM =>
                                DATA_B_WRITE_DONE <= '0';
                                DATA_B_READ_DONE  <= '1';
                                bram_rd <= '1';
                                ccl_fifo_wr <= '0';
                                bram_addrb <= equivalence;
                            when WRITE_BRAM =>
                                DATA_B_WRITE_DONE <= '1';
                                DATA_B_READ_DONE  <= '0';
                                ccl_reg <= bram_doutb;
                                ccl_fifo_wr <= '1';
                            when others =>
                                bram_wr <= "0";
                                bram_rd <= '0';
                                ccl_fifo_wr <= '0'; 
                                bram_addrb <= (others => '0');
                                bram_addra <= (others => '0');
                                bram_dina <= (others => '0');                               
                        end case;
                    when others => NULL;
                end case;
            when SECOND_SCAN =>
                ccl_fifo_wr <= '1';
                bram_rd <= '1';
                bram_addrb <= equivalence;
                ccl_reg <= bram_doutb;
            when others => NULL;
            end case;
    end if;
end process;
first_scan_result_fifo :FIFO_RTL  
    generic map ( 
        g_WIDTH  => label_bits,
        g_DEPTH  => img_width*img_height+10   
    )  
    port map(
        i_clk => i_clk,
        i_rst => i_rst,
        i_wr_en => SECOND_SCAN_START,
        i_rd_en => ccl_fifo_wr,
        i_data => local_min_label,
        o_data => equivalence
    );
second_scan_result_fifo: FIFO_RTL
    generic map(
        g_WIDTH  => label_bits,
        g_DEPTH  => img_width
    )
    port map(
        i_clk   =>  i_clk,
        i_rst   =>  i_rst,                
        i_wr_en =>  ccl_fifo_wr,
        i_rd_en =>  SECOND,
        i_data  =>  ccl_reg,
        o_data  =>  o_ccl
    );
o_debug_1 <= equivalence;
IP_BRAM :blk_mem_gen_1 
    port map(
        clka            => i_clk,
        ena             => '1',    
        wea             => bram_wr,
        dina            => bram_dina,
        addra           => bram_addra,
        clkb            => i_clk,
        enb             => bram_rd,
        addrb           => bram_addrb,
        doutb           => bram_doutb
    );
end Behavioral;