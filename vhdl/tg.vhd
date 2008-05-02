-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TG module -- 4 input mux
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity tgslice is
  
  port (
    i1, i2, i3, i4 : in  std_logic;           -- inputs
    a, b, c, d     : in  std_logic;           -- selectors
    q, qb          : out std_logic;           -- outputs
    tp             : out std_logic);          -- test point

end tgslice;

architecture gates of tgslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal ii1, ii2, ii3, ii4 : std_logic;
  signal ti : std_logic;                      -- internal copy of output
begin  -- gates
  u1 : g2 port map (
    a => i1,
    b => a,
    y => ii1);
  u2 : g2 port map (
    a => i2,
    b => b,
    y => ii2);
  u3 : g2 port map (
    a => i3,
    b => c,
    y => ii3);
  u4 : g2 port map (
    a => i4,
    b => d,
    y => ii4);
  u5 : g4 port map (
    a  => ii1,
    b  => ii2,
    c  => ii3,
    d  => ii4,
    y  => ti,
    y2 => qb);
  tp <= ti;
  q <= ti;
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity tg is
  
  port (
    p2, p4, p6, p8     : in  std_logic;   -- bit 0 inputs
    p13, p14           : out std_logic;   -- bit 0 outputs (true/complement)
    p1, p3, p5, p7     : in  std_logic;  -- bit 1 inputs
    p9, p10            : out std_logic;  -- bit 1 outputs
    p28, p26, p24, p22 : in  std_logic;  -- bit 2 inputs
    p17, p18           : out std_logic;  -- bit 2 outputs
    p27, p25, p23, p21 : in  std_logic;  -- bit 3 inputs
    p19, p20           : out std_logic;  -- bit 3 outputs
    tp1, tp2, tp5, tp6 : out std_logic;   -- test points
    p12, p16, p11, p15 : in  std_logic);  -- selects a, b, c, d

end tg;

architecture gates of tg is
  component inv
    port (
      a : in  std_logic;                      -- input
      y : out std_logic);                     -- output
  end component;
  component tgslice
    port (
      i1, i2, i3, i4 : in  std_logic;           -- inputs
      a, b, c, d     : in  std_logic;           -- selectors
      q, qb          : out std_logic;           -- outputs
      tp             : out std_logic);          -- test point
  end component;
  signal a, b, c, d : std_logic;        -- internal selects
begin  -- gates

  u1 : inv port map (
    a => p12,
    y => a);
  u2 : inv port map (
    a => p16,
    y => b);
  u3 : inv port map (
    a => p11,
    y => c);
  u4 : inv port map (
    a => p15,
    y => d);
  u5 : tgslice port map (
    i1 => p2,
    i2 => p4,
    i3 => p6,
    i4 => p8,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    q  => p13,
    qb => p14,
    tp => tp1);
  u6 : tgslice port map (
    i1 => p1,
    i2 => p3,
    i3 => p5,
    i4 => p7,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    tp => tp2,
    q  => p10,
    qb => p9);
  u7 : tgslice port map (
    i1 => p28,
    i2 => p26,
    i3 => p24,
    i4 => p22,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    tp => tp5,
    q  => p17,
    qb => p18);
  u8 : tgslice port map (
    i1 => p27,
    i2 => p25,
    i3 => p23,
    i4 => p21,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    tp => tp6,
    q  => p20,
    qb => p19);
end gates;
