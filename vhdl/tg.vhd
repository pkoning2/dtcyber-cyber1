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
-- TG module, rev H -- 4 input mux
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity tgslice is
  
  port (
    i1, i2, i3, i4 : in  logicsig;           -- inputs
    a, b, c, d     : in  logicsig;           -- selectors
    q, qb          : out logicsig;           -- outputs
    tp             : out logicsig);          -- test point

end tgslice;

architecture gates of tgslice is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  logicsig;             -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal ti1, ti2, ti3, ti4 : logicsig;
  signal ti : logicsig;                      -- internal copy of output
begin  -- gates
  u1 : g2 port map (
    a => i1,
    b => a,
    y => ti1);
  u2 : g2 port map (
    a => i2,
    b => b,
    y => ti2);
  u3 : g2 port map (
    a => i3,
    b => c,
    y => ti3);
  u4 : g2 port map (
    a => i4,
    b => d,
    y => ti4);
  u5 : g4 port map (
    a  => ti1,
    b  => ti2,
    c  => ti3,
    d  => ti4,
    y  => ti,
    y2 => qb);
  tp <= ti;
  q <= ti;
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tg is
  
  port (
    p2, p4, p6, p8     : in  logicsig;   -- bit 0 inputs
    p13, p14           : out logicsig;   -- bit 0 outputs (true/complement)
    p1, p3, p5, p7     : in  logicsig;  -- bit 1 inputs
    p9, p10            : out logicsig;  -- bit 1 outputs
    p28, p26, p24, p22 : in  logicsig;  -- bit 2 inputs
    p17, p18           : out logicsig;  -- bit 2 outputs
    p27, p25, p23, p21 : in  logicsig;  -- bit 3 inputs
    p19, p20           : out logicsig;  -- bit 3 outputs
    tp1, tp2, tp5, tp6 : out logicsig;   -- test points
    p12, p16, p11, p15 : in  logicsig);  -- selects a, b, c, d

end tg;

architecture gates of tg is
  component inv
    port (
      a : in  logicsig;                      -- input
      y : out logicsig);                     -- output
  end component;
  component tgslice
    port (
      i1, i2, i3, i4 : in  logicsig;           -- inputs
      a, b, c, d     : in  logicsig;           -- selectors
      q, qb          : out logicsig;           -- outputs
      tp             : out logicsig);          -- test point
  end component;
  signal a, b, c, d : logicsig;        -- internal selects
begin  -- gates

  u1 : inv port map (
    a => p12,
    y => a);
  u2 : inv port map (
    a => p16,
    y => b);
  u3 : inv port map (
    a => p11,
    y => c);
  u4 : inv port map (
    a => p15,
    y => d);
  u5 : tgslice port map (
    i1 => p2,
    i2 => p4,
    i3 => p6,
    i4 => p8,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    q  => p13,
    qb => p14,
    tp => tp1);
  u6 : tgslice port map (
    i1 => p1,
    i2 => p3,
    i3 => p5,
    i4 => p7,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    tp => tp2,
    q  => p10,
    qb => p9);
  u7 : tgslice port map (
    i1 => p28,
    i2 => p26,
    i3 => p24,
    i4 => p22,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    tp => tp5,
    q  => p17,
    qb => p18);
  u8 : tgslice port map (
    i1 => p27,
    i2 => p25,
    i3 => p23,
    i4 => p21,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    tp => tp6,
    q  => p20,
    qb => p19);
end gates;
