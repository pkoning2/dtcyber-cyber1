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
-- QB module, rev E
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qb is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end qb;
architecture gates of qb is
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
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;
  signal t18 : logicsig;
  signal t19 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p7,
    b => p5,
    y => c);


  u2 : inv2 port map (
    a => p3,
    y => f,
    y2 => t1);


  u3 : g2 port map (
    a => p17,
    b => p19,
    y => b);


  u4 : inv2 port map (
    a => p15,
    y => e,
    y2 => t2);


  u5 : g2 port map (
    a => p11,
    b => p9,
    y => a);


  u6 : inv2 port map (
    a => p13,
    y => d,
    y2 => t3);


  u7 : g2 port map (
    a => c,
    b => t1,
    y => t4);

  p4 <= t4;
  tp2 <= t4;

  u8 : g2 port map (
    a => b,
    b => t2,
    y => t5);

  p8 <= t5;
  tp3 <= t5;

  u9 : g2 port map (
    a => a,
    b => t3,
    y => t6);

  p6 <= t6;
  tp4 <= t6;

  u10 : g3 port map (
    a => p21,
    b => p25,
    c => p27,
    y => x);


  u11 : inv2 port map (
    a => x,
    y => p18,
    y2 => t7);

  p26 <= t7;
  tp6 <= t7;

  u12 : g2 port map (
    a => a,
    b => x,
    y => t8);


  u13 : inv port map (
    a => d,
    y => t9);


  u14 : g2 port map (
    a => t8,
    b => t9,
    y => t10,
    y2 => p1);

  p2 <= t10;
  tp1 <= t10;

  u15 : g3 port map (
    a => a,
    b => b,
    c => c,
    y2 => t11);

  p20 <= t11;
  p22 <= t11;
  p24 <= t11;

  u16 : g2 port map (
    a => e,
    b => c,
    y => t12);


  u17 : inv port map (
    a => f,
    y => t13);


  u18 : g3 port map (
    a => c,
    b => b,
    c => d,
    y => t14);


  u19 : g3 port map (
    a => t12,
    b => t13,
    c => t14,
    y => t15);

  p14 <= t15;
  p16 <= t15;

  u20 : g3 port map (
    a => x,
    b => a,
    c => b,
    y => t16);


  u21 : g2 port map (
    a => d,
    b => b,
    y => t17);


  u22 : inv port map (
    a => e,
    y => t18);


  u23 : g3 port map (
    a => t18,
    b => t16,
    c => t17,
    y => t19,
    y2 => p12);

  p28 <= t19;
  tp5 <= t19;


end gates;

