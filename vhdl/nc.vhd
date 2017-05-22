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
-- NC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nc is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p2_tp1 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p14_p16_tp3 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19_p27_p13 : out logicsig;
      p25_tp6 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end nc;
architecture gates of nc is
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
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t20 : logicsig;
  signal t21 : logicsig;
  signal t22 : logicsig;
  signal t23 : logicsig;
  signal t24 : logicsig;
  signal t25 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p6,
    y => t1,
    y2 => t2);


  u2 : g2 port map (
    a => t2,
    b => g,
    y => t3);


  u3 : g3 port map (
    a => t3,
    b => g,
    c => p3,
    y => t4);

  p9 <= t4;

  u4 : g2 port map (
    a => t1,
    b => d);

  tp2 <= d;

  u5 : g3 port map (
    a => t1,
    b => d,
    c => e,
    y => h);

  tp5 <= e;

  u6 : g2 port map (
    a => t3,
    b => h,
    y => t6);


  u7 : g2 port map (
    a => t6,
    b => p1,
    y => t7);

  p7 <= t7;

  u8 : g2 port map (
    a => t4,
    b => t7,
    y => p2_tp1);


  u9 : inv2 port map (
    a => p23,
    y => t10,
    y2 => t11);


  u10 : g2 port map (
    a => g,
    b => t11,
    y => t12);

  p19_p27_p13 <= g;

  u11 : g3 port map (
    a => t12,
    b => i,
    c => p22,
    y => t13);

  p28 <= t13;

  u12 : g3 port map (
    a => t10,
    b => d,
    c => e,
    y => i);


  u13 : g2 port map (
    a => t12,
    b => i,
    y => t14);


  u14 : g2 port map (
    a => t14,
    b => p24,
    y => t15);

  p26 <= t15;

  u15 : g2 port map (
    a => t13,
    b => t15,
    y => p25_tp6);


  u16 : inv2 port map (
    a => p12,
    y => t20,
    y2 => t21);


  u17 : g2 port map (
    a => t20,
    b => p20,
    y => t22);


  u18 : inv port map (
    a => p20,
    y => t23);


  u19 : g2 port map (
    a => t23,
    b => t21,
    y => t24);


  u20 : g2 port map (
    a => t22,
    b => t24,
    y => p14_p16_tp3);


  u21 : g3 port map (
    a => t20,
    b => a,
    c => c,
    y => e);

  p11 <= c;
  p17 <= a;

  u22 : g2 port map (
    a => t21,
    b => b,
    y => d);

  tp4 <= b;

  u23 : g2 port map (
    a => d,
    b => e,
    y => g);


  u25 : inv port map (
    a => g,
    y => t25);

  p8 <= t25;

  u26 : inv2 port map (
    a => t25,
    y => p10,
    y2 => p18);


  u27 : g2 port map (
    a => p21,
    b => p15,
    y => a);


  u28 : inv port map (
    a => p5,
    y => c);


  u29 : g2 port map (
    a => a,
    b => c,
    y => b);



end gates;

