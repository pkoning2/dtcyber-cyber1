-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- JR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jrslice is
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      a : out logicsig;
      f : out logicsig);

end jrslice;
architecture gates of jrslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => i1,
    y => t1,
    y2 => t2);


  u2 : g2 port map (
    a => t1,
    b => i2,
    y => t3);


  u3 : g2 port map (
    a => t2,
    b => i3,
    y => t4);


  u4 : g2 port map (
    a => t3,
    b => t4,
    y => a,
    y2 => f);



end gates;

use work.sigs.all;

entity jr is
    port (
      p1 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p9 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p24 : out logicsig);

end jr;
architecture gates of jr is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component jrslice
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      a : out logicsig;
      f : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : jrslice port map (
    i1 => p7,
    i2 => p10,
    i3 => p8,
    a => b,
    f => g);

  tp1 <= b;

  u2 : jrslice port map (
    i1 => p6,
    i2 => p1,
    i3 => p4,
    a => a,
    f => f);

  p2 <= f;
  tp2 <= f;

  u3 : jrslice port map (
    i1 => p11,
    i2 => p12,
    i3 => p19,
    a => c,
    f => h);


  u4 : jrslice port map (
    i1 => p22,
    i2 => p21,
    i3 => p28,
    a => d,
    f => i);

  tp6 <= d;

  u5 : jrslice port map (
    i1 => p25,
    i2 => p26,
    i3 => p27,
    a => e,
    f => j);

  tp5 <= e;

  u6 : g5 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    y2 => t1);

  p9 <= t1;
  tp3 <= t1;

  u7 : g5 port map (
    a => f,
    b => g,
    c => h,
    d => i,
    e => j,
    y => t2);

  p24 <= t2;
  tp4 <= t2;

  u8 : g2 port map (
    a => p17,
    b => p20,
    y2 => t3);

  p13 <= t3;
  p14 <= t3;
  p15 <= t3;
  p18 <= t3;


end gates;

