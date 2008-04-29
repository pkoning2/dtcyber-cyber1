-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TD module -- quad 5-output fanout
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

entity td is
  
  port (
    p1, p3, p2, p4, p25, p27, p26, p28 : in  std_logic;
    tp1, tp2, tp5, tp6                 : out std_logic;  -- test points
    p6, p8, p10, p12, p14              : out std_logic;
    p5, p7, p9, p11, p13               : out std_logic;
    p16, p18, p20, p22, p24            : out std_logic;
    p15, p17, p19, p21, p23            : out std_logic);

end td;
architecture gates of td is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t5, t6 : std_logic := '0';
begin  -- gates

  u1 : g2 port map (
    a  => p1,
    b  => p3,
    y  => tp1,
    y2 => t1);
  p6 <= t1;
  p8 <= t1;
  p10 <= t1;
  p12 <= t1;
  p14 <= t1;
  u2 : g2 port map (
    a  => p2,
    b  => p4,
    y  => tp2,
    y2 => t2);
  p5 <= t2;
  p7 <= t2;
  p9 <= t2;
  p11 <= t2;
  p13 <= t2;
  u3 : g2 port map (
    a  => p25,
    b  => p27,
    y  => tp5,
    y2 => t5);
  p16 <= t5;  
  p18 <= t5;  
  p20 <= t5;  
  p22 <= t5;  
  p24 <= t5;  
  u4 : g2 port map (
    a  => p26,
    b  => p28,
    y  => tp6,
    y2 => t6);
  p15 <= t6;
  p17 <= t6;
  p19 <= t6;
  p21 <= t6;
  p23 <= t6;
  
end gates;


