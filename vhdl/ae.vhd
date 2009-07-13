-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- AE module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity aeslice is
  
  port (
    a, s1, s2 : in  std_logic;
    r1, r2    : in  std_logic;
    r3        : in  std_logic;
    tp        : out std_logic;          -- test point
    q, qb     : out std_logic);

end aeslice;

architecture gates of aeslice is
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      s2, s3, s4, r2, r3, r4  : in  std_logic := '1';-- extra set, reset if needed
      q, qb : out std_logic);                 -- q and q.bar
  end component;
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g3 port map (
    a => a,
    b => s1,
    c => s2,
    y => t1);
  u2 : g3 port map (
    a => r1,
    b => r2,
    c => a,
    y => t2);
  u3 : rsflop port map (
    s  => t1,
    r  => t2,
    r2 => r3,
    q  => t4,
    qb => qb);
  tp <= t4;
  q <= t4;

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ae is
  
  port (
    p3, p5, p2, p4     : in  std_logic;
    p11, p7, p14, p12  : in  std_logic;
    p15, p13, p24, p22 : in  std_logic;
    p21, p23, p28, p26 : in  std_logic;
    p16, p19, p17      : in  std_logic;
    tp1, tp2, tp5, tp6 : out std_logic;  -- test points
    p8, p1, p10, p9    : out std_logic;
    p20, p18, p25, p27 : out std_logic);

end ae;

architecture gates of ae is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component aeslice
    port (
      a, s1, s2 : in  std_logic;
      r1, r2    : in  std_logic;
      r3        : in  std_logic;
      tp        : out std_logic;          -- test point
      q, qb     : out std_logic);
  end component;
  signal a, c : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p16,
    y2 => a);
  u2 : g2 port map (
    a  => p19,
    b  => p17,
    y2 => c);
  u3 : aeslice port map (
    a  => a,
    s1 => p3,
    s2 => p5,
    r1 => p2,
    r2 => p4,
    r3 => c,
    tp => tp1,
    q  => p8,
    qb => p1);
  u4 : aeslice port map (
    a  => a,
    s1 => p11,
    s2 => p7,
    r1 => p14,
    r2 => p12,
    r3 => c,
    tp => tp2,
    q  => p10,
    qb => p9);
  u5 : aeslice port map (
    a  => a,
    s1 => p15,
    s2 => p13,
    r1 => p24,
    r2 => p22,
    r3 => c,
    tp => tp5,
    q  => p20,
    qb => p18);
  u6 : aeslice port map (
    a  => a,
    s1 => p21,
    s2 => p23,
    r1 => p28,
    r2 => p26,
    r3 => c,
    tp => tp6,
    q  => p25,
    qb => p27);
  
end gates;
