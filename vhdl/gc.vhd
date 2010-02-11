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
-- GC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gc is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p13_tp4 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21_tp3 : out logicsig;
      p28 : out logicsig);

end gc;
architecture gates of gc is
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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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

  component rs4flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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
  signal k : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : rs4flop port map (
    r => a,
    s => p2,
    s2 => p3,
    s3 => p4,
    s4 => p5,
    q => t1);

  tp1 <= t1;

  u2 : inv port map (
    a => t1,
    y => j);

  p1 <= j;

  u3 : inv2 port map (
    a => p6,
    y => t2,
    y2 => k);


  u4 : g2 port map (
    a => j,
    b => k,
    y => g);

  p8 <= g;

  u5 : g2 port map (
    a => t1,
    b => t2,
    y => f);

  p7 <= f;

  u6 : g2 port map (
    a => f,
    b => g,
    y => p9);


  u7 : rs4flop port map (
    r => a,
    s => p22,
    s2 => p20,
    s3 => p18,
    s4 => p16,
    q => t3);

  tp5 <= t3;

  u8 : inv2 port map (
    a => p10,
    y => t4,
    y2 => i);


  u9 : inv port map (
    a => t3,
    y => h);

  p15 <= h;

  u10 : g2 port map (
    a => h,
    b => i,
    y => e,
    y2 => p17);


  u11 : g2 port map (
    a => t3,
    b => t4,
    y => d);


  u12 : g2 port map (
    a => d,
    b => e,
    y => p11);


  u13 : rs4flop port map (
    r => a,
    s => p27,
    s2 => p26,
    s3 => p23,
    s4 => p24,
    q => t5);

  tp6 <= t5;

  u14 : inv port map (
    a => p12,
    y => a);


  u15 : inv2 port map (
    a => p25,
    y => t6,
    y2 => t7);


  u16 : inv port map (
    a => t5,
    y => t8);

  p28 <= t8;

  u17 : g2 port map (
    a => t8,
    b => t7,
    y => c);


  u18 : g2 port map (
    a => t5,
    b => t6,
    y => b);


  u19 : g2 port map (
    a => b,
    b => c,
    y => p14);


  u20 : g3 port map (
    a => b,
    b => d,
    c => f,
    y => p13_tp4,
    y2 => p19);


  u21 : g4 port map (
    a => b,
    b => d,
    c => k,
    d => j,
    y => t10);

  tp2 <= t10;

  u22 : g3 port map (
    a => h,
    b => i,
    c => b,
    y => t11);


  u23 : g3 port map (
    a => t10,
    b => c,
    c => t11,
    y => p21_tp3);



end gates;

