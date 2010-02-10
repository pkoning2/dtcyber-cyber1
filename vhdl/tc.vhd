-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
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

use work.sigs.all;

entity tc is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end tc;
architecture gates of tc is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
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

