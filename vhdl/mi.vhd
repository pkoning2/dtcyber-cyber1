-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- MI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mi is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p1_tp2 : out logicsig;
      p3_tp1 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13_tp5 : out logicsig;
      p14_tp3 : out logicsig;
      p15_tp4 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p22_tp6 : out logicsig;
      p24_p26 : out logicsig;
      p28 : out logicsig);

end mi;
architecture gates of mi is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
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
  signal i : logicsig;

begin -- gates
  u1 : g6 port map (
    a => g,
    b => h,
    c => i,
    d => p6,
    e => p4,
    f => p2,
    y => p3_tp1,
    y2 => p9);

  p10 <= h;

  u2 : g3 port map (
    a => g,
    b => h,
    c => i,
    y => p1_tp2);


  u3 : g6 port map (
    a => d,
    b => e,
    c => f,
    d => g,
    e => h,
    f => i,
    y => p14_tp3,
    y2 => p11);

  p16 <= e;

  u4 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => p15_tp4);

  p18 <= a;
  p20 <= c;
  p24_p26 <= b;

  u5 : g3 port map (
    a => d,
    b => e,
    c => f,
    y => p13_tp5);


  u6 : g6 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    y => p22_tp6,
    y2 => p28);


  u7 : inv port map (
    a => p7,
    y => g);


  u8 : inv port map (
    a => p8,
    y => h);


  u9 : inv port map (
    a => p5,
    y => i);


  u10 : inv port map (
    a => p23,
    y => d);


  u11 : inv port map (
    a => p17,
    y => e);


  u12 : inv port map (
    a => p19,
    y => f);


  u13 : inv port map (
    a => p21,
    y => a);


  u14 : inv port map (
    a => p27,
    y => b);


  u15 : inv port map (
    a => p25,
    y => c);



end gates;

