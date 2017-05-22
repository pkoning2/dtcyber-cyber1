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
-- KF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      tp1 : out logicsig;
      tp4 : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end kfslice;
architecture gates of kfslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => i1,
    b => i2,
    c => i3,
    y => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => a,
    b => t1,
    y => y1);


  u3 : g2 port map (
    a => t1,
    b => b,
    y => t2);


  u4 : g3 port map (
    a => i4,
    b => t2,
    c => i5,
    y => t3);

  tp4 <= t3;

  u5 : g2 port map (
    a => c,
    b => t3,
    y => y2);


  u6 : g2 port map (
    a => t3,
    b => d,
    y => y3);



end gates;

use work.sigs.all;

entity kf is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p7 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end kf;
architecture gates of kf is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component kfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      tp1 : out logicsig;
      tp4 : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv port map (
    a => p9,
    y => a);


  u2 : inv port map (
    a => p3,
    y => b);


  u3 : inv port map (
    a => p19,
    y => c);


  u4 : inv port map (
    a => p24,
    y => d);


  u5 : kfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p8,
    i2 => p5,
    i3 => p10,
    i4 => p18,
    i5 => p17,
    tp1 => tp1,
    tp4 => tp4,
    y1 => p7,
    y2 => p15,
    y3 => p13);


  u6 : kfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p2,
    i2 => p4,
    i3 => p6,
    i4 => p26,
    i5 => p28,
    tp1 => tp2,
    tp4 => tp5,
    y1 => p1,
    y2 => p27,
    y3 => p21);


  u7 : kfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p12,
    i2 => p14,
    i3 => p16,
    i4 => p22,
    i5 => p20,
    tp1 => tp3,
    tp4 => tp6,
    y1 => p11,
    y2 => p23,
    y3 => p25);



end gates;

