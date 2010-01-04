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
-- QT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qt is
    port (
      p13 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
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
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig);

end qt;
architecture gates of qt is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal k : logicsig;
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

begin -- gates
  u1 : inv port map (
    a => p13,
    y => t1);


  u2 : inv2 port map (
    a => t1,
    y => a,
    y2 => b);

  tp3 <= a;

  u3 : inv port map (
    a => p14,
    y => t2);


  u4 : inv2 port map (
    a => t2,
    y => c,
    y2 => d);

  tp2 <= c;

  u5 : inv port map (
    a => p18,
    y => t3);


  u6 : inv2 port map (
    a => t3,
    y => e,
    y2 => f);

  tp5 <= e;

  u7 : g4 port map (
    a => p26,
    b => p25,
    c => p28,
    d => p27,
    y2 => k);


  u8 : g3 port map (
    a => p13,
    b => p14,
    c => p18,
    y2 => t4);

  p3 <= t4;
  p5 <= t4;
  p7 <= t4;

  u9 : inv2 port map (
    a => k,
    y => t5,
    y2 => t6);


  u10 : g2 port map (
    a => t5,
    b => a,
    y => t7);


  u11 : g2 port map (
    a => b,
    b => t6,
    y => t8);


  u12 : g2 port map (
    a => t7,
    b => t8,
    y => t9);

  p9 <= t9;
  tp1 <= t9;

  u13 : inv2 port map (
    a => t9,
    y2 => p10);


  u14 : g2 port map (
    a => a,
    b => k,
    y => t10,
    y2 => t11);


  u15 : g2 port map (
    a => t10,
    b => c,
    y => t12);


  u17 : g2 port map (
    a => t11,
    b => d,
    y => t13);


  u18 : g2 port map (
    a => t12,
    b => t13,
    y => t14);

  p24 <= t14;
  tp4 <= t14;

  u19 : inv2 port map (
    a => t14,
    y2 => p21);


  u20 : g3 port map (
    a => a,
    b => c,
    c => k,
    y => t15,
    y2 => t16);


  u21 : g2 port map (
    a => t15,
    b => e,
    y => t17);


  u22 : g2 port map (
    a => t16,
    b => f,
    y => t18);


  u23 : g2 port map (
    a => t17,
    b => t18,
    y => t19);

  p20 <= t19;
  tp6 <= t19;

  u24 : inv2 port map (
    a => t19,
    y2 => p23);



end gates;

