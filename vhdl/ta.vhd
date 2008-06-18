-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- TA module, rev C -- 6 input gates
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ta is
  
  port (
    p14, p12, p10, p8, p6, p4    : in  std_logic;
    p13, p11, p9, p7, p5, p3     : in  std_logic;
    p16, p18, p20, p22, p24, p26 : in  std_logic;
    p15, p17, p19, p21, p23, p25 : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p2, p1, p28, p27             : out std_logic);

end ta;

architecture gates of ta is
  component g6
    port (
      a, b, c, d, e, f : in  std_logic;       -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g6 port map (
    a => p14,
    b => p12,
    c => p10,
    d => p8,
    e => p6,
    f => p4,
    y => t1);
  tp1 <= t1;
  p2 <= t1;
  u2 : g6 port map (
    a => p13,
    b => p11,
    c => p9,
    d => p7,
    e => p5,
    f => p3,
    y => t2);
  tp2 <= t2;
  p1 <= t2;
  u3 : g6 port map (
    a => p16,
    b => p18,
    c => p20,
    d => p22,
    e => p24,
    f => p26,
    y => t3);
  tp5 <= t3;
  p28 <= t3;
  u4 : g6 port map (
    a => p15,
    b => p17,
    c => p19,
    d => p21,
    e => p23,
    f => p25,
    y => t4);
  tp6 <= t4;
  p27 <= t4;

end gates;
