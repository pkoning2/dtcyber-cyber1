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
-- TC module, rev D -- dual 12 output fanout
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity tc is
  
  port (
    p3, p5, p14, p16            : in  std_logic;  -- inputs
    tp1, tp5                    : out std_logic;  -- test point 1 (stage 1)
    tp2, tp6                    : out std_logic;  -- test point 2 (stage 2)
    p1, p2, p4, p6, p7, p8      : out std_logic;  -- outputs 1
    p9, p10, p11, p12, p13, p15 : out std_logic;  -- outputs 1
    p17, p18, p19, p20, p21, p22 : out std_logic;  -- outputs 2
    p23, p24, p25, p26, p27, p28 : out std_logic);  -- outputs 2

end tc;

architecture gates of tc is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2    : out std_logic);                  -- output
  end component;
  signal a, b, c, d : std_logic;
begin  -- gates
  u1 : g2 port map (
    a => p3,
    b => p5,
    y => tp1,
    y2 => a);
  tp2 <= a;
  u2 : inv port map (
    a => a,
    y => b);
  p1 <= b;
  p2 <= b;
  p4 <= b;
  p6 <= b;
  p7 <= b;
  p8 <= b;
  p9 <= b;
  p10 <= b;
  p11 <= b;
  p12 <= b;
  p13 <= b;
  p15 <= b;
  u4 : g2 port map (
    a => p14,
    b => p16,
    y => tp5,
    y2 => c);
  tp6 <= c;
  u5 : inv port map (
    a => c,
    y => d);
  p17 <= d;
  p18 <= d;
  p19 <= d;
  p20 <= d;
  p21 <= d;
  p22 <= d;
  p23 <= d;
  p24 <= d;
  p25 <= d;
  p26 <= d;
  p27 <= d;
  p28 <= d;

end gates;

