-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

entity pgslice is
  
  port (
    dbit, abit : in  std_logic;               -- d(n) and a(n) input bits
    a, b, d, e : in  std_logic;               -- common enables/clocks
    tp         : out std_logic;               -- test point
    q          : out std_logic);              -- output

end pgslice;

architecture gates of pgslice is
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      x       : out std_logic);               -- output
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      q, qb : out std_logic);                 -- outputs
  end component;
  signal s, s1, s2, ti : std_logic;           -- intermediate values
begin  -- gates

  u1 : g3 port map (
    a => e,
    b => dbit,
    c => b,
    x => s1);
  u2 : g3 port map (
    a => b,
    b => abit,
    c => d,
    x => s2);
  s <= s1 or s2;
  u3 : rsflop port map (
    s => s,
    r => a,
    q => ti);
  tp <= ti;
  u4 : inv port map (
    i => ti,
    o => q);
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;

entity pg is
  
  port (
    p5  : in  std_logic;                      -- d0
    p3  : in  std_logic;                      -- a0
    p10 : in  std_logic;                      -- clock I
    p6  : in  std_logic;                      -- III (pre-slot)
    p9  : in  std_logic;                      -- 72x...
    p21 : in  std_logic;                      -- 76x...
    p7  : in  std_logic;                      -- 732...
    p23 : in  std_logic;                      -- 77x...
    tp1 : out std_logic;                      -- test point 1
    p13 : out std_logic);                     -- data 0

end pg;

architecture gates of pg is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      x    : out std_logic);                  -- output
  end component;
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  component pgslice
    port (
      dbit, abit : in  std_logic;               -- d(n) and a(n) input bits
      a, b, d, e : in  std_logic;               -- common enables/clocks
      tp         : out std_logic;               -- test point
      q          : out std_logic);              -- output
  end component;
  signal a, bi, b, d, e : std_logic;          -- control signals
begin  -- gates

  u1 : inv port map (
    i => p10,
    o => a);
  u2 : inv port map (
    i => p6,
    o => bi);
  u3 : inv port map (
    i => bi,
    o => b);
  u4 : g2 port map (
    a => p9,
    b => p21,
    x => d);
  u5 : g2 port map (
    a => p7,
    b => p23,
    x => e);
  u6 : pgslice port map (
    dbit => p5,
    abit => p3,
    a    => a,
    b    => b,
    d    => d,
    e    => e,
    tp   => tp1,
    q    => p13);

end gates;

