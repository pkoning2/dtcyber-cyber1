-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- TF module, rev B
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tfslice is
  
  port (
    i1, i2, i3, i4 : in  std_logic;     -- inputs
    a, b, c, d     : in  std_logic;     -- selects
    e              : in  std_logic;
    y              : out std_logic);

end tfslice;

architecture gates of tfslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component g5
    port (
      a, b, c, d, e : in  std_logic;          -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => i1,
    b => a,
    y => t1);
  u2 : g2 port map (
    a => i2,
    b => b,
    y => t2);
  u3 : g2 port map (
    a => i3,
    b => c,
    y => t3);
  u4 : g2 port map (
    a => i4,
    b => d,
    y => t4);
  u5 : g5 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => e,
    y => y);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tf is
  
  port (
    p8, p22, p7, p21, p6, p24, p5, p23 : in  std_logic;
    p4, p26, p3, p25, p1, p27, p2, p28 : in  std_logic;
    p9, p10, p20                       : in  std_logic;
    tp1, tp2, tp5, tp6                 : out std_logic;  -- test points
    p13, p11, p19                      : out std_logic);

end tf;

architecture gates of tf is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component tfslice
    port (
      i1, i2, i3, i4 : in  std_logic;     -- inputs
      a, b, c, d     : in  std_logic;     -- selects
      e              : in  std_logic;
      y              : out std_logic);
  end component;
  signal a, b, c, d, e, f, g, t1 : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p1,
    y => a);
  u2 : inv port map (
    a => p27,
    y => b);
  u3 : inv port map (
    a => p2,
    y => c);
  u4 : inv port map (
    a => p28,
    y => d);
  u5 : tfslice port map (
    i1 => p8,
    i2 => p22,
    i3 => p7,
    i4 => p21,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    e  => p9,
    y  => e);
  tp5 <= e;
  u6 : tfslice port map (
    i1 => p6,
    i2 => p24,
    i3 => p5,
    i4 => p23,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    e  => p20,
    y  => f);
  tp6 <= f;
  u7 : tfslice port map (
    i1 => p4,
    i2 => p26,
    i3 => p3,
    i4 => p25,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    e  => p10,
    y  => g);
  u8 : g3 port map (
    a => e,
    b => f,
    c => g,
    y => t1);
  p13 <= t1;
  tp1 <= t1;
  p11 <= t1;
  p19 <= t1;

end gates;
