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
-- TH module, rev C -- assorted fanouts
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity th is
  
  port (
    p1, p3, p6, p25, p26, p28  : in  logicsig;  -- inputs
    tp1, tp2, tp3, tp4, tp5, tp6 : out logicsig;  -- test point
    p7, p8, p14, p2, p4, p10, p12: out logicsig;  -- outputs
    p16, p18, p20, p24, p5, p9, p11, p13 : out logicsig;  -- outputs
    p17, p19, p23, p27, p15, p21, p22 : out logicsig);  -- outputs

end th;

architecture gates of th is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  signal t1, t2, t3, t4, t5, t6 : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => p1,
    y  => tp1,
    y2 => t1);
  p14 <= t1;
  p8 <= t1;
  p7 <= t1;
  u2 : inv2 port map (
    a  => p6,
    y  => tp2,
    y2 => t2);
  p5 <= t2;
  p9 <= t2;
  p11 <= t2;
  p13 <= t2;
  u3 : inv2 port map (
    a  => p3,
    y  => tp3,
    y2 => t3);
  p2 <= t3;
  p12 <= t3;
  p10 <= t3;
  p4 <= t3;
  u4 : inv2 port map (
    a  => p26,
    y  => tp4,
    y2 => t4);
  p23 <= t4;
  p19 <= t4;
  p17 <= t4;
  p27 <= t4;
  u5 : inv2 port map (
    a  => p25,
    y  => tp5,
    y2 => t5);
  p16 <= t5;
  p18 <= t5;
  p20 <= t5;
  p24 <= t5;
  u6 : inv2 port map (
    a  => p28,
    y  => tp6,
    y2 => t6);
  p22 <= t6;
  p21 <= t6;
  p15 <= t6;
end gates;

