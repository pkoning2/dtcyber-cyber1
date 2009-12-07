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
-- QH module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qhslice is
  
  port (
    a, c : in  logicsig;
    b, d : in  logicsig := '1';
    e    : in  logicsig := '1';
    tp   : out logicsig;               -- test point
    y    : out logicsig);              -- output

end qhslice;

architecture gates of qhslice is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2, t3 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);
  u2 : g2 port map (
    a => c,
    b => d,
    y => t2);
  u3 : g3 port map (
    a => t1,
    b => t2,
    c => e,
    y => t3);
  tp <= t3;
  y <= t3;
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qh is
  
  port (
    p10, p12, p5, p7, p9         : in  logicsig;
    p2, p4, p6, p8               : in  logicsig;
    p13, p15, p17, p14           : in  logicsig;
    p21, p23, p25, p27, p26      : in  logicsig;
    p18, p20, p22, p24           : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6 : out logicsig;   -- test points
    p3, p11, p1, p16, p28, p19   : out logicsig);  -- outputs

end qh;

architecture gates of qh is
  component qhslice
    port (
      a, c : in  logicsig;
      b, d : in  logicsig := '1';
      e    : in  logicsig := '1';
      tp   : out logicsig;               -- test point
      y    : out logicsig);              -- output
  end component;
begin  -- gates

  u1 : qhslice port map (
    a  => p10,
    c  => p12,
    e  => p5,
    tp => tp1,
    y  => p3);
  u2 : qhslice port map (
    a  => p7,
    c  => p9,
    tp => tp2,
    y  => p11);
  u3 : qhslice port map (
    a  => p2,
    b  => p4,
    c  => p6,
    d  => p8,
    tp => tp3,
    y  => p1);
  u4 : qhslice port map (
    a  => p13,
    b  => p15,
    c  => p17,
    e  => p14,
    tp => tp4,
    y  => p16);
  u5 : qhslice port map (
    a  => p21,
    b  => p23,
    c  => p25,
    d  => p27,
    e  => p26,
    tp => tp5,
    y  => p28);
  u6 : qhslice port map (
    a  => p18,
    b  => p20,
    c  => p22,
    d  => p24,
    tp => tp6,
    y  => p19);

end gates;
