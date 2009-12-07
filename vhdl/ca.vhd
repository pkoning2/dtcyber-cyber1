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
-- CA module -- monster fanout
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ca is
  
  port (
    p12, p14, p16                                    : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6                     : out logicsig;
    p10, p8, p6, p4, p2, p9, p7, p5, p3, p1          : out logicsig;
    p28, p26, p24, p22, p20, p27, p25, p23, p21, p19 : out logicsig;
    p17, p18, p15, p13, p11                          : out logicsig);

end ca;

architecture gates of ca is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2 : logicsig;
begin  -- gates

  u1 : g3 port map (
    a  => p12,
    b  => p14,
    c  => p16,
    y  => tp2,
    y2 => t1);
  tp1 <= t1;
  tp3 <= t1;
  tp4 <= t1;
  tp5 <= t1;
  tp6 <= t1;
  u2 : inv port map (
    a => t1,
    y => t2);
  p10 <= t2;
  p8 <= t2;
  p6 <= t2;
  p4 <= t2;
  p2 <= t2;
  p9 <= t2;
  p7 <= t2;
  p5 <= t2;
  p3 <= t2;
  p1 <= t2;
  p28 <= t2;
  p26 <= t2;
  p24 <= t2;
  p22 <= t2;
  p20 <= t2;
  p27 <= t2;
  p25 <= t2;
  p23 <= t2;
  p21 <= t2;
  p19 <= t2;
  p17 <= t2;
  p18 <= t2;
  p15 <= t2;
  p13 <= t2;
  p11 <= t2;
  
end gates;
