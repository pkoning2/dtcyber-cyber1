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
-- PV module, rev D
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pv is
  
  port (
    p27, p25, p14, p12, p5, p10     : in    logicsig;
    p6, p2, p4, p3, p8              : in    logicsig;
    tp1, tp2, tp5, tp6              : out   logicsig;  -- test points
    p1, p13, p28, p17, p7, p11, p19 : out   logicsig;
    p16, p18, p20, p22, p24, p26    : inout misc);

end pv;

architecture gates of pv is
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
  signal a, b, c, d, e, f, g, h : logicsig;
  signal i, j, k, l, m : logicsig;
  signal t1, t2, t3, t4, t5, t6, t7 : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => p27,
    y  => a,
    y2 => b);
  u2 : inv2 port map (
    a  => p25,
    y  => c,
    y2 => d);
  u3 : inv2 port map (
    a  => p14,
    y  => e,
    y2 => f);
  u4 : inv2 port map (
    a  => p12,
    y  => g,
    y2 => h);
  u5 : inv2 port map (
    a  => p5,
    y  => i,
    y2 => j);
  u6 : inv2 port map (
    a  => p10,
    y  => k,
    y2 => l);
  u7 : g2 port map (
    a => l,
    b => p6,
    y => t1);
  u8 : g4 port map (
    a => k,
    b => j,
    c => g,
    d => e,
    y => t2);
  u9 : g2 port map (
    a => h,
    b => j,
    y => t3);
  u10 : g5 port map (
    a => t1,
    b => p2,
    c => p4,
    d => t2,
    e => t3,
    y => t4);
  tp1 <= t4;
  u11 : g2 port map (
    a => p3,
    b => t4,
    y => p1);
  u12 : g4 port map (
    a  => h,
    b  => p8,
    c  => i,
    d  => k,
    y2 => m);
  u13 : g4 port map (
    a  => m,
    b  => a,
    c  => c,
    d  => e,
    y  => tp2,
    y2 => p13);
  u14 : g3 port map (
    a => m,
    b => e,
    c => b,
    y => t5);
  u15 : g3 port map (
    a => m,
    b => e,
    c => d,
    y => t6);
  tp6 <= t6;
  p17 <= t6;
  u16 : g2 port map (
    a  => t5,
    b  => t6,
    y  => tp5,
    y2 => p28);
  u17 : g3 port map (
    a => m,
    b => f,
    c => c,
    y => p7);
  u18 : g2 port map (
    a => m,
    b => a,
    y => p11);
  u19 : g2 port map (
    a => m,
    b => b,
    y => p19);
  
end gates;
