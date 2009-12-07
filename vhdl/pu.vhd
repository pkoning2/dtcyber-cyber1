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
-- PU module, rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity puslice is
  
  port (
    s, r, e : in  logicsig;
    tp      : out logicsig;            -- test point
    q1, q2  : out logicsig;            -- q outputs
    qb      : out logicsig);           -- qb output

end puslice;

architecture gates of puslice is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal tri, tsi, tqi, tqbi : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => s,
    b => e,
    y => tsi);
  u2 : g2 port map (
    a => r,
    b => e,
    y => tri);
  u3 : rsflop port map (
    r  => tri,
    s  => tsi,
    q  => tqi,
    qb => tqbi);
  tp <= tqi;
  u4 : inv2 port map (
    a  => tqi,
    y  => q1,
    y2 => q2);
  u5 : inv2 port map (
    a  => tqbi,
    y2 => qb);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pu is
  
  port (
    p10, p11, p17, p19, p18           : in  logicsig;
    tp1, tp5, tp6                     : out logicsig;  -- test points
    p9, p8, p1, p4, p3, p6, p7        : out logicsig;
    p14, p16, p15                     : inout misc;  -- analog stuff
    p22, p21, p28, p27, p26, p25, p24 : out logicsig);

end pu;

architecture gates of pu is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component puslice
    port (
      s, r, e : in  logicsig;
      tp      : out logicsig;            -- test point
      q1, q2  : out logicsig;            -- q outputs
      qb      : out logicsig);           -- qb output
  end component;
  signal a, t1, t2 : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => p17,
    y2 => a);
  tp5 <= a;
  u2 : puslice port map (
    s  => p10,
    r  => p11,
    e  => a,
    tp => tp1,
    q1 => p9,
    q2 => p8,
    qb => t1);
  p1 <= t1;
  p4 <= t1;
  p3 <= t1;
  p6 <= t1;
  p7 <= t1;
  u3 : puslice port map (
    s  => p19,
    r  => p18,
    e  => a,
    tp => tp6,
    q1 => p22,
    q2 => p21,
    qb => t2);
  p28 <= t2;
  p27 <= t2;
  p26 <= t2;
  p25 <= t2;
  p24 <= t2;
  
end gates;
