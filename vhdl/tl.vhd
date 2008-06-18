-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- TL module -- assorted gates and inverters
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tl is
  
  port (
    p8, p2, p4, p6, p5, p7       : in  std_logic;
    p1, p3, p22, p21, p23, p24   : in  std_logic;
    p25, p26, p27, p28           : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;   -- test points
    p12, p10, p11, p9, p14, p19  : out std_logic;
    p13, p16, p15, p18, p17, p20 : out std_logic);

end tl;

architecture gates of tl is
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
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3, t4, t5, t6 : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p8,
    y => t1);
  tp1 <= t1;
  p12 <= t1;
  u2 : g3 port map (
    a => p2,
    b => t4,
    c => p6,
    y => t2);
  tp2 <= t2;
  p10 <= t2;
  u3 : g2 port map (
    a => p5,
    b => p7,
    y => t3);
  tp3 <= t3;
  p11 <= t3;
  u4 : g2 port map (
    a => p1,
    b => p3,
    y => t4);
  tp4 <= t4;
  p9 <= t4;
  u5 : inv port map (
    a => p22,
    y => t5);
  tp5 <= t5;
  p14 <= t5;
  u6 : inv port map (
    a => p21,
    y => t6);
  tp6 <= t6;
  p19 <= t6;
  u7 : inv port map (
    a => p23,
    y => p13);
  u8 : inv port map (
    a => p24,
    y => p16);
  u9 : inv port map (
    a => p25,
    y => p15);
  u10 : inv port map (
    a => p26,
    y => p18);
  u11 : inv port map (
    a => p27,
    y => p17);
  u12 : inv port map (
    a => p28,
    y => p20);
  
end gates;
