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
-- AR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ar is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p11 : out logicsig;
      p12_tp3 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ar;
architecture gates of ar is
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
  signal f : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal m : logicsig;
  signal r : logicsig;
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
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p22,
    y => c);


  u2 : inv2 port map (
    a => p6,
    y => a,
    y2 => p1);


  u3 : g2 port map (
    a => p22,
    b => a,
    y => r);


  u4 : g2 port map (
    a => p9,
    b => a,
    y => p11,
    y2 => p2);


  u5 : inv port map (
    a => p3,
    y => t1);


  u6 : g3 port map (
    a => p8,
    b => t1,
    c => p10,
    y => k);

  tp1 <= k;

  u7 : inv port map (
    a => p23,
    y => d);


  u8 : inv port map (
    a => p20,
    y => b);


  u9 : g2 port map (
    a => a,
    b => p23,
    y => t2);


  u11 : g3 port map (
    a => d,
    b => b,
    c => c,
    y => t3);

  tp4 <= t3;

  u12 : g3 port map (
    a => k,
    b => t3,
    c => p5,
    y => t4,
    y2 => p18);


  u13 : g3 port map (
    a => r,
    b => t2,
    c => t4,
    y => p4);


  u14 : g2 port map (
    a => t2,
    b => t4,
    y2 => p19);


  u15 : inv port map (
    a => p13,
    y => m);


  u16 : inv port map (
    a => p14,
    y => t5);


  u17 : g2 port map (
    a => m,
    b => t5,
    y2 => p15);


  u18 : g2 port map (
    a => a,
    b => t5,
    y => t6);


  u19 : g2 port map (
    a => j,
    b => h,
    y => t7);

  tp5 <= t7;

  u20 : g2 port map (
    a => t7,
    b => k,
    y => t8);


  u21 : g2 port map (
    a => t6,
    b => t8,
    y => p16);


  u22 : inv port map (
    a => p7,
    y => t9);


  u23 : g2 port map (
    a => t9,
    b => d,
    y => t10);


  u24 : g2 port map (
    a => m,
    b => t10,
    y2 => p12_tp3);


  u25 : inv port map (
    a => p21,
    y => t12);


  u26 : g2 port map (
    a => b,
    b => t12,
    y => t13);


  u27 : g2 port map (
    a => m,
    b => t13,
    y2 => p17);


  u28 : inv port map (
    a => p24,
    y => h);


  u29 : g2 port map (
    a => h,
    b => c,
    y => t14);

  tp6 <= t14;

  u30 : g2 port map (
    a => m,
    b => t14,
    y2 => p26);


  u31 : inv port map (
    a => p27,
    y => f);


  u32 : inv port map (
    a => p25,
    y => j);


  u33 : g2 port map (
    a => f,
    b => j,
    y => t15);


  u34 : g2 port map (
    a => m,
    b => t15,
    y2 => p28);



end gates;

