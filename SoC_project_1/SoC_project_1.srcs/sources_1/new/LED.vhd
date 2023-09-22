----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 2023/09/21 15:21:28
-- Design Name: 
-- Module Name: LED - Behavioral
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
use ieee.std_logic_signed.all;
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx leaf cells in this code.
--library UNISIM;
--use UNISIM.VComponents.all;

entity LED is
    Port (
            rst : in std_logic;
            clk : in std_logic;            
            led_o : out std_logic_vector(7 downto 0)
    );
end LED;

architecture Behavioral of LED is

component divider is
    generic(
        DIV_CLK_CONSTANT : integer
    );
    Port ( clk : in STD_LOGIC;
           rst : in STD_LOGIC;
           div_clk : out STD_LOGIC);
end component;

signal div_clk : std_logic;
signal led : std_logic_vector(7 downto 0);
begin
    divider_1: divider
        generic map(
            DIV_CLK_CONSTANT => 35000000
        )
        Port map( 
           clk => clk,
           rst => rst,
           div_clk => div_clk
       );
    led_process:process(clk , rst)begin
        if rst = '1' then
            led <= (others => '0');
        elsif rising_edge(div_clk) then
            led <= led + 1;
        end if;
    end process;
    led_o <= led;
end Behavioral;
