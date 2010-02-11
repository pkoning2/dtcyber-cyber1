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
-- HK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hk is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p7 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3_p14 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p20_p26_p28 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p27 : out logicsig);

end hk;
architecture gates of hk is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
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
  signal l : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p2,
    b => g,
    y => t1);

  tp2 <= g;

  u2 : g2 port map (
    a => p2,
    b => e,
    y => t2);

  tp5 <= e;

  u3 : g2 port map (
    a => t1,
    b => t2,
    y => l,
    y2 => t3);


  u4 : inv port map (
    a => t3,
    y => p8);


  u5 : rsflop port map (
    r => p18,
    s => t1,
    q => a,
    qb => b);

  tp1 <= a;

  u6 : g2 port map (
    a => c,
    b => a,
    y => p27);

  tp6 <= c;

  u7 : inv port map (
    a => a,
    y => p24);


  u8 : g2 port map (
    a => a,
    b => d,
    y => p6);


  u9 : rsflop port map (
    r => p18,
    s => t2,
    q => c,
    qb => d);


  u10 : inv port map (
    a => h,
    y => t4);


  u11 : g2 port map (
    a => p12,
    b => t4,
    y => p16);


  u12 : g2 port map (
    a => t4,
    b => l,
    y => p22);


  u13 : g4 port map (
    a => b,
    b => d,
    c => p4,
    d => p25,
    y2 => t5);


  u14 : g2 port map (
    a => p19,
    b => t5,
    y => t6);


  u15 : g3 port map (
    a => t5,
    b => p13,
    c => p11,
    y => t7);


  u16 : g3 port map (
    a => p21,
    b => t5,
    c => p7,
    y => h);


  u17 : g2 port map (
    a => h,
    b => t7,
    y => g);


  u18 : g2 port map (
    a => t6,
    b => h,
    y => e);


  u19 : g3 port map (
    a => t6,
    b => h,
    c => t7,
    y => f,
    y2 => p5);

  p10 <= f;

  u20 : g2 port map (
    a => f,
    b => p23,
    y => p9,
    y2 => p17);


  u21 : g2 port map (
    a => p25,
    b => p15,
    y2 => p20_p26_p28);


  u22 : g2 port map (
    a => b,
    b => d,
    y => t9);


  u24 : g2 port map (
    a => p15,
    b => t9,
    y2 => p1_p3_p14);



end gates;

