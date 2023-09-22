library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity divider is
    generic(
        DIV_CLK_CONSTANT : integer
    );
    Port ( clk : in STD_LOGIC;
           rst : in STD_LOGIC;
           div_clk : out STD_LOGIC);
end divider;


architecture Behavioral of divider is

signal cnt  :  integer;
signal div_clk_sig : std_logic;

begin
div_clk <= div_clk_sig;
div_process : process(clk,rst,cnt)
begin
    if rst = '1' then
        div_clk_sig <= '0';
    else
        if rising_edge(clk)then
            if cnt = DIV_CLK_CONSTANT then
                div_clk_sig <= not div_clk_sig;
            elsif cnt = DIV_CLK_CONSTANT/2 then
                div_clk_sig <= not div_clk_sig;
            end if;
        end if;
    end if;
end process;

cnt_process : process(clk,rst,cnt)
begin
    if rst = '1' then
        cnt <= 0;
    else
        if rising_edge(clk)then
            if cnt < DIV_CLK_CONSTANT then
                cnt <= cnt + 1 ;
            else
                cnt <= 0 ;
            end if;
        end if;
    end if;
end process;

end Behavioral;
