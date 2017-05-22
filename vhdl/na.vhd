-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- NA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity naslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end naslice;
architecture gates of naslice is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : g4 port map (
    a => i1,
    b => i2,
    c => i3,
    d => i4,
    y => t1);

  tp <= t1;

  u2 : g3 port map (
    a => a,
    b => t1,
    c => c,
    y => y1);


  u3 : g3 port map (
    a => c,
    b => t1,
    c => b,
    y => y2);



end gates;

use work.sigs.all;

entity na is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end na;
architecture gates of na is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component naslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p14,
    y2 => c);

  tp3 <= c;

  u2 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);

  tp4 <= a;

  u3 : naslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p1,
    i2 => p3,
    i3 => p5,
    i4 => p6,
    tp => tp1,
    y1 => p4,
    y2 => p2);


  u4 : naslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p7,
    i2 => p8,
    i3 => p11,
    i4 => p9,
    tp => tp2,
    y1 => p10,
    y2 => p12);


  u5 : naslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p21,
    i2 => p15,
    i3 => p17,
    i4 => p19,
    tp => tp5,
    y1 => p22,
    y2 => p20);


  u6 : naslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p24,
    i2 => p23,
    i3 => p27,
    i4 => p25,
    tp => tp6,
    y1 => p26,
    y2 => p28);



end gates;

