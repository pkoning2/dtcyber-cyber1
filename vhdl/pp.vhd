-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PP module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pp is
  
  port (
    p19, p16, p21, p1, p3, p12, p9, p7, p20        : in  std_logic;
    p2, p6, p8, p11, p23, p27, p28, p15            : in  std_logic;
    tp1, tp2, tp5, tp6                             : out std_logic;  -- test points
    p14, p4, p5, p10, p13, p18, p22, p25, p26, p17 : out std_logic);

end pp;

architecture gates of pp is
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
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal a : std_logic;
  signal t1, t2, t3, t4, t5, t6, t7, t8 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => p1,
    b => p3,
    y => t1);
  u2 : g2 port map (
    a => p12,
    b => p9,
    y => t2);
  u3 : inv port map (
    a => p7,
    y => t3);
  u4 : g3 port map (
    a  => t1,
    b  => t2,
    c  => t3,
    y  => a,
    y2 => t4);
  u5 : inv port map (
    a => p16,
    y => t5);
  tp1 <= a;
  u6 : g4 port map (
    a  => p19,
    b  => t5,
    c  => p21,
    d  => t4,
    y  => tp6,
    y2 => p14);
  u7 : g2 port map (
    a => p2,
    b => a,
    y => p4);
  u8 : g2 port map (
    a => p6,
    b => a,
    y => p5);
  u9 : g2 port map (
    a => p8,
    b => a,
    y => p10);
  u10 : g2 port map (
    a => a,
    b => p11,
    y => t7);
  tp2 <= t7;
  p13 <= t7;
  u11 : g2 port map (
    a => p20,
    b => a,
    y => p18);
  u12 : g2 port map (
    a => p23,
    b => a,
    y => p22);
  u13 : g2 port map (
    a => p27,
    b => a,
    y => p25);
  u14 : g2 port map (
    a => p28,
    b => a,
    y => p26);
  u15 : g2 port map (
    a => a,
    b => p15,
    y => t8);
  tp5 <= t8;
  p17 <= t8;

end gates;
