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
-- IH module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ih is
  
  port (
    p6, p4, p24, p26                       : in  logicsig;
    p1, p2, p3, p5                         : in  logicsig;
    p23, p25, p27, p28                     : in  logicsig;
    tp1, tp2, tp5, tp6                     : out logicsig;  -- test points
    p13, p14, p11, p12, p9, p10, p7, p8    : out logicsig;
    p21, p22, p19, p20, p15, p16, p17, p18 : out logicsig);

end ih;

architecture gates of ih is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal a, b, c, d : logicsig;
begin  -- gates

  u1 : inv port map (
    a => p6,
    y => a);
  tp1 <= a;
  u2 : inv port map (
    a => p4,
    y => b);
  tp2 <= b;
  u3 : inv port map (
    a => p24,
    y => c);
  tp5 <= c;
  u4 : inv port map (
    a => p26,
    y => d);
  tp6 <= d;
  u5 : g2 port map (
    a => a,
    b => p1,
    y => p13);
  u6 : g2 port map (
    a => p1,
    b => b,
    y => p14);
  u7 : g2 port map (
    a => a,
    b => p2,
    y => p11);
  u8 : g2 port map (
    a => p2,
    b => b,
    y => p12);
  u9 : g2 port map (
    a => a,
    b => p3,
    y => p9);
  u10 : g2 port map (
    a => p3,
    b => b,
    y => p10);
  u11 : g2 port map (
    a => a,
    b => p5,
    y => p7);
  u12 : g2 port map (
    a => p5,
    b => b,
    y => p8);
  u13 : g2 port map (
    a => c,
    b => p23,
    y => p21);
  u14 : g2 port map (
    a => p23,
    b => d,
    y => p22);
  u15 : g2 port map (
    a => c,
    b => p25,
    y => p19);
  u16 : g2 port map (
    a => p25,
    b => d,
    y => p20);
  u17 : g2 port map (
    a => c,
    b => p27,
    y => p15);
  u18 : g2 port map (
    a => p27,
    b => d,
    y => p16);
  u19 : g2 port map (
    a => c,
    b => p28,
    y => p17);
  u20 : g2 port map (
    a => p28,
    b => d,
    y => p18);

end gates;
