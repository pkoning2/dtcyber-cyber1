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
-- TB module, rev C -- 3-6 input gates
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tb is
  
  port (
    p1, p3, p5, p7, p9, p11 : in  logicsig;
    p2, p4, p6, p8, p10     : in  logicsig;
    p21, p23, p25, p27      : in  logicsig;
    p24, p26, p28           : in  logicsig;
    tp1, tp2, tp5, tp6      : out logicsig;  -- test points
    p19, p13, p12, p15, p17 : out logicsig;
    p14, p16, p18, p20, p22 : out logicsig);

end tb;

architecture gates of tb is
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  logicsig;             -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g5
    port (
      a, b, c, d, e : in  logicsig;          -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g6
    port (
      a, b, c, d, e, f : in  logicsig;       -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2, t3 : logicsig;
begin  -- gates

  u1 : g6 port map (
    a  => p1,
    b  => p3,
    c  => p5,
    d  => p7,
    e  => p9,
    f  => p11,
    y  => tp1,
    y2 => p19);
  u2 : g5 port map (
    a  => p2,
    b  => p4,
    c  => p6,
    d  => p8,
    e  => p10,
    y  => tp2,
    y2 => t1);
  p13 <= t1;
  p12 <= t1;
  p15 <= t1;
  p17 <= t1;
  u3 : g4 port map (
    a  => p21,
    b  => p23,
    c  => p25,
    d  => p27,
    y  => tp5,
    y2 => t2);
  p14 <= t2;
  p16 <= t2;
  p18 <= t2;
  u4 : g3 port map (
    a  => p24,
    b  => p26,
    c  => p28,
    y  => tp6,
    y2 => t3);
  p20 <= t3;
  p22 <= t3;
  
end gates;
