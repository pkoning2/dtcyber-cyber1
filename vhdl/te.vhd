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
-- TE module, rev C -- hex 3-input mux
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity teslice is
  
  port (
    a, b, c    : in  logicsig;         -- inputs
    e1, e2, e3 : in  logicsig;         -- enables
    tp         : out logicsig;         -- test point
    y          : out logicsig);        -- output

end teslice;

architecture gates of teslice is
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
  signal t1, t2, t3, t4 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => e1,
    y => t1);
  u2 : g2 port map (
    a => b,
    b => e2,
    y => t2);
  u3 : g2 port map (
    a => c,
    b => e3,
    y => t3);
  u4 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => t4);
  tp <= t4;
  y <= t4;

end gates;
use work.sigs.all;

entity te is
  
  port (
    p6, p4, p2, p7, p13, p12, p24, p26, p28  : in  logicsig;
    p5, p3, p1, p22, p18, p17, p23, p25, p27 : in  logicsig;
    p14, p15, p16                            : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6             : out logicsig;  -- test points
    p9, p10, p21, p8, p19, p20               : out logicsig);

end te;

architecture gates of te is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component teslice
    port (
      a, b, c    : in  logicsig;         -- inputs
      e1, e2, e3 : in  logicsig;         -- enables
      tp         : out logicsig;         -- test point
      y          : out logicsig);        -- output
  end component;
  signal a, b, c : logicsig;
begin  -- gates

  u1 : inv port map (
    a => p14,
    y => a);
  u2 : inv port map (
    a => p15,
    y => b);
  u3 : inv port map (
    a => p16,
    y => c);
  u4 : teslice port map (
    a  => p6,
    b  => p4,
    c  => p2,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp1,
    y  => p9);
  u5 : teslice port map (
    a  => p5,
    b  => p3,
    c  => p1,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp2,
    y  => p8);
  u6 : teslice port map (
    a  => p7,
    b  => p13,
    c  => p12,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp3,
    y  => p10);
  u7 : teslice port map (
    a  => p22,
    b  => p18,
    c  => p17,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp4,
    y  => p19);
  u8 : teslice port map (
    a  => p24,
    b  => p26,
    c  => p28,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp5,
    y  => p21);
  u9 : teslice port map (
    a  => p23,
    b  => p25,
    c  => p27,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp6,
    y  => p20);

end gates;

  
