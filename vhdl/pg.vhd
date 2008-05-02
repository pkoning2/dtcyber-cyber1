-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PG module - storage address register (G)
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

entity pgslice is
  
  port (
    i1, i2     : in  std_logic;               -- input bits
    a, b, d, e : in  std_logic;               -- common enables/clocks
    tp         : out std_logic;               -- test point
    q          : out std_logic);              -- output

end pgslice;

architecture gates of pgslice is
  component inv
    port (
      a : in  std_logic;                      -- input
      y : out std_logic);                     -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2       : out std_logic);               -- output
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      s2, s3, s4, r2, r3, r4  : in  std_logic;  -- extra set, reset if needed
      q, qb : out std_logic);                 -- outputs
  end component;
  signal s, s1, s2, ti : std_logic;           -- intermediate values
begin  -- gates

  u1 : g3 port map (
    a => e,
    b => i1,
    c => b,
    y => s1);
  u2 : g3 port map (
    a => b,
    b => i2,
    c => d,
    y => s2);
  s <= s1 or s2;
  u3 : rsflop port map (
    s => s,
    r => a,
    q => ti);
  tp <= ti;
  u4 : inv port map (
    a => ti,
    y => q);
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity pg is

  port (
    p10, p8, p9, p21, p7, p23 : in  std_logic;  -- enables
    p5, p3, p6, p4            : in  std_logic;  -- inputs 1 and 2
    p25, p27, p26, p28        : in  std_logic;  -- inputs 3 and 4
    p13, p14, p17, p20        : out std_logic;  -- outputs
    tp1, tp2, tp5, tp6        : out std_logic);  -- test points

end pg;

architecture gates of pg is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2    : out std_logic);                  -- output
  end component;
  component inv
    port (
      a : in  std_logic;                      -- input
      y : out std_logic);                     -- output
  end component;
  component inv2
    port (
      a : in  std_logic;                      -- input
      y, y2 : out std_logic);                     -- output
  end component;
  component pgslice
    port (
      i1, i2     : in  std_logic;               -- input bits
      a, b, d, e : in  std_logic;               -- common enables/clocks
      tp         : out std_logic;               -- test point
      q          : out std_logic);              -- output
  end component;
  signal a, b, d, e : std_logic;          -- control signals
begin  -- gates

  u1 : inv port map (
    a => p10,
    y => a);
  u2 : inv2 port map (
    a => p6,
    y2 => b);
  u4 : g2 port map (
    a => p9,
    b => p21,
    y => d);
  u5 : g2 port map (
    a => p7,
    b => p23,
    y => e);
  u6 : pgslice port map (
    i1 => p5,
    i2 => p3,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp1,
    q  => p13);
  u7 : pgslice port map (
    i1 => p6,
    i2 => p4,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp2,
    q  => p14);
  u8 : pgslice port map (
    i1 => p25,
    i2 => p27,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp5,
    q  => p17);
  u9 : pgslice port map (
    i1 => p26,
    i2 => p28,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp6,
    q  => p20);

end gates;

