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
-- IW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity iw is
    port (
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end iw;
architecture gates of iw is
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
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p18,
    y => a);

  p19 <= a;

  u2 : inv port map (
    a => p20,
    y => b);

  p21 <= b;

  u3 : inv port map (
    a => p22,
    y => c);

  p23 <= c;

  u4 : inv port map (
    a => p24,
    y => d);

  p25 <= d;

  u5 : inv port map (
    a => p13,
    y => e);

  p12 <= e;

  u6 : inv port map (
    a => p11,
    y => f);

  p10 <= f;
  tp1 <= f;

  u7 : inv port map (
    a => p9,
    y => g);

  p8 <= g;

  u8 : inv port map (
    a => p7,
    y => h);

  p6 <= h;

  u9 : g2 port map (
    a => p15,
    b => p16,
    y => t1,
    y2 => t2);

  p1 <= t1;
  tp5 <= t1;

  u10 : g2 port map (
    a => t2,
    b => e,
    y => t3);

  p14 <= t3;
  tp2 <= t3;

  u11 : g2 port map (
    a => t2,
    b => f,
    y => p4);


  u12 : g2 port map (
    a => t2,
    b => g,
    y => p5);


  u13 : g2 port map (
    a => t2,
    b => h,
    y => p3);


  u14 : g2 port map (
    a => t2,
    b => a,
    y => t4);

  p17 <= t4;
  tp6 <= t4;

  u15 : g2 port map (
    a => t2,
    b => b,
    y => p27);


  u16 : g2 port map (
    a => t2,
    b => c,
    y => p26);


  u17 : g2 port map (
    a => t2,
    b => d,
    y => p28);



end gates;

