-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- HC module -- 10 way fanout
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity hc is
  
  port (
    p7, p5, p23, p25        : in  std_logic;
    tp1, tp2, tp5, tp6      : out std_logic;  -- test points
    p2, p4, p6, p9, p11     : out std_logic;
    p13, p8, p10, p12, p14  : out std_logic;
    p16, p18, p20, p22, p17 : out std_logic;
    p19, p21, p24, p26, p28 : out std_logic);

end hc;

architecture gates of hc is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a  => p7,
    b  => p5,
    y  => t1,
    y2 => t2);
  tp1 <= t1;
  tp2 <= t1;
  p2<= t2;
  p4<= t2;
  p6<= t2;
  p9<= t2;
  p11<= t2;
  p13<= t2;
  p8<= t2;
  p10<= t2;
  p12<= t2;
  p14 <= t2;
  u2 : g2 port map (
    a  => p23,
    b  => p25,
    y  => t3,
    y2 => t4);
  tp5 <= t3;
  tp6 <= t3;
  p16<= t4;
  p18<= t4;
  p20<= t4;
  p22<= t4;
  p17<= t4;
  p19<= t4;
  p21<= t4;
  p24<= t4;
  p26<= t4;
  p28<= t4;
  
end gates;
