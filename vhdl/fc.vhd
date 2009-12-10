-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- FC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity fc is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p24 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end fc;
architecture gates of fc is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p1,
    y => a);

  p2 <= a;

  u2 : inv port map (
    a => p3,
    y => b);

  p4 <= b;

  u3 : inv port map (
    a => p7,
    y => c);

  p8 <= c;

  u4 : inv port map (
    a => p9,
    y => d);

  p10 <= d;

  u5 : inv2 port map (
    a => p13,
    y => e,
    y2 => p11);


  u6 : g2 port map (
    a => a,
    b => b,
    y => p6,
    y2 => g);


  u7 : g2 port map (
    a => b,
    b => c,
    y => p5,
    y2 => h);


  u8 : g2 port map (
    a => c,
    b => d,
    y => t1);

  p12 <= t1;
  tp1 <= t1;

  u9 : g2 port map (
    a => d,
    b => e,
    y => p15);


  u10 : g2 port map (
    a => p26,
    b => p23,
    y2 => j);


  u11 : g3 port map (
    a => p26,
    b => p23,
    c => p25,
    y2 => k);


  u12 : g2 port map (
    a => e,
    b => a,
    y2 => i);

  p27 <= i;
  tp3 <= i;

  u13 : g3 port map (
    a => i,
    b => h,
    c => p21,
    y => t2);

  tp5 <= t2;

  u14 : g2 port map (
    a => p22,
    b => i,
    y => t3);

  tp6 <= t3;

  u15 : inv port map (
    a => p14,
    y => t4);


  u16 : g2 port map (
    a => p19,
    b => e,
    y => t5);


  u17 : g3 port map (
    a => p20,
    b => g,
    c => e,
    y => t6);


  u18 : g5 port map (
    a => t2,
    b => t3,
    c => t4,
    d => t5,
    e => t6,
    y => t7,
    y2 => p16);

  tp4 <= t7;

  u19 : g2 port map (
    a => p17,
    b => t7,
    y => p18);


  u20 : g2 port map (
    a => t7,
    b => j,
    y => p24);


  u21 : g2 port map (
    a => t7,
    b => k,
    y => p28);



end gates;

