-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- CB module -- 6 way fanout and big fanout
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity cb is
  
  port (
    p7, p8, p16, p14             : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p4, p5, p6, p1, p2, p3       : out std_logic;
    p11, p12, p10, p9, p17, p20  : out std_logic;
    p19, p27, p22, p23, p21, p24 : out std_logic;
    p13, p15, p18, p25, p26, p28 : out std_logic);

end cb;

architecture gates of cb is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a  => p7,
    b  => p8,
    y  => tp2,
    y2 => t1);
  tp1 <= t1;
  tp3 <= t1;
  u2 : inv port map (
    a => t1,
    y => t2);
  p4 <= t2;
  p5 <= t2;
  p6 <= t2;
  p1 <= t2;
  p2 <= t2;
  p3 <= t2;
  u3 : g2 port map (
    a  => p16,
    b  => p14,
    y  => tp5,
    y2 => t3);
  tp4 <= t3;
  tp6 <= t3;
  u4 : inv port map (
    a => t3,
    y => t4);
  p11<= t4;
  p12<= t4;
  p10<= t4;
  p9<= t4;
  p17<= t4;
  p20<= t4;
  p19<= t4;
  p27<= t4;
  p22<= t4;
  p23<= t4;
  p21<= t4;
  p24<= t4;
  p13<= t4;
  p15<= t4;
  p18<= t4;
  p25<= t4;
  p26<= t4;
  p28<= t4;

end gates;
