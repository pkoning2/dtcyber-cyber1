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
-- QS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qs is
    port (
      p2 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4_p8_tp1 : out logicsig;
      p7_tp2 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p17 : out logicsig;
      p21_p25_tp6 : out logicsig;
      p26_tp5 : out logicsig;
      p28 : out logicsig);

end qs;
architecture gates of qs is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal f : logicsig;
  signal j : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal p : logicsig;
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
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t17 : logicsig;
  signal t18 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p2,
    y => t1,
    y2 => t2);


  u2 : inv2 port map (
    a => p6,
    y => c,
    y2 => b);


  u3 : g2 port map (
    a => t1,
    b => b,
    y => n);


  u4 : g2 port map (
    a => t2,
    b => c,
    y => m);


  u5 : g2 port map (
    a => n,
    b => m,
    y => p);

  p4_p8_tp1 <= p;

  u6 : g2 port map (
    a => p,
    b => f,
    y => p1);


  u7 : inv2 port map (
    a => p27,
    y => t3,
    y2 => t4);


  u8 : inv2 port map (
    a => p23,
    y => a,
    y2 => t5);


  u9 : g2 port map (
    a => t3,
    b => t5,
    y => t6);


  u10 : g2 port map (
    a => t4,
    b => a,
    y => t7);


  u11 : g2 port map (
    a => t6,
    b => t7,
    y => t8);

  p21_p25_tp6 <= t8;

  u12 : g2 port map (
    a => t8,
    b => f,
    y => p28);


  u13 : g2 port map (
    a => a,
    b => c,
    y2 => p26_tp5);


  u14 : inv2 port map (
    a => p12,
    y2 => f);


  u15 : g3 port map (
    a => p,
    b => c,
    c => p18,
    y => t10);


  u16 : g2 port map (
    a => p18,
    b => p20,
    y => t11);


  u17 : g4 port map (
    a => n,
    b => c,
    c => p22,
    d => m,
    y => t12);


  u18 : g2 port map (
    a => p22,
    b => p24,
    y => t13);


  u19 : g6 port map (
    a => t10,
    b => t11,
    c => t12,
    d => t13,
    e => p9,
    f => p19,
    y => t14);


  u20 : g2 port map (
    a => t14,
    b => p5,
    y => p3);


  u21 : inv port map (
    a => p16,
    y => j);


  u22 : inv port map (
    a => p10,
    y => t15);

  p11 <= t15;

  u23 : g2 port map (
    a => j,
    b => t15,
    y => p7_tp2);


  u24 : inv port map (
    a => p14,
    y => t17);


  u25 : g2 port map (
    a => t17,
    b => j,
    y => p17);


  u26 : inv port map (
    a => p15,
    y => t18);


  u27 : g2 port map (
    a => t18,
    b => j,
    y => p13);



end gates;

