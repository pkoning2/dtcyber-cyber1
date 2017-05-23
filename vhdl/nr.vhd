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
-- NR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nr is
    port (
      p4 : in  logicsig;
      p10 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p19 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p6_tp1 : out logicsig;
      p8 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p17_tp3 : out logicsig;
      p18 : out logicsig;
      p20_tp4 : out logicsig;
      p22_tp5 : out logicsig;
      p23_tp6 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end nr;
architecture gates of nr is
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
  signal g : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal s : logicsig;
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
  signal u : logicsig;
  signal v : logicsig;
  signal w : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p14,
    y => a,
    y2 => b);

  p16 <= p14;

  u2 : inv2 port map (
    a => p15,
    y => c,
    y2 => d);

  p13 <= p15;

  u3 : inv2 port map (
    a => p19,
    y => e,
    y2 => f);


  u4 : inv2 port map (
    a => p10,
    y => g,
    y2 => h);


  u5 : g3 port map (
    a => b,
    b => d,
    c => f,
    y => t1);

  p11 <= t1;

  u6 : g2 port map (
    a => t1,
    b => h,
    y => p17_tp3);


  u7 : g2 port map (
    a => g,
    b => b,
    y => s);


  u8 : g2 port map (
    a => g,
    b => d,
    y => v);

  tp2 <= v;

  u9 : g2 port map (
    a => s,
    b => u,
    y => k);

  p3 <= k;

  u10 : g2 port map (
    a => w,
    b => v,
    y => p5);


  u11 : g3 port map (
    a => a,
    b => c,
    c => e,
    y => t2);


  u12 : g2 port map (
    a => g,
    b => t2,
    y => p18);


  u13 : g2 port map (
    a => h,
    b => e,
    y => t3);


  u14 : g2 port map (
    a => f,
    b => g,
    y => t4);


  u15 : g2 port map (
    a => t3,
    b => t4,
    y => p20_tp4);


  u16 : inv port map (
    a => p27,
    y => t5);


  u17 : g2 port map (
    a => t5,
    b => a,
    y => p28);


  u18 : g2 port map (
    a => p4,
    b => k,
    y => p2);


  u19 : inv port map (
    a => p4,
    y => t6);

  p8 <= t6;

  u20 : g2 port map (
    a => a,
    b => h,
    y => u);


  u21 : g2 port map (
    a => t6,
    b => u,
    y => j);


  u22 : g2 port map (
    a => j,
    b => s,
    y => p12);


  u23 : g2 port map (
    a => p27,
    b => b,
    y => t7,
    y2 => t8);

  p25 <= t7;

  u24 : g2 port map (
    a => c,
    b => t7,
    y => p26);


  u25 : g2 port map (
    a => t8,
    b => d,
    y => t9,
    y2 => t10);

  p24 <= t9;

  u26 : g2 port map (
    a => t9,
    b => e,
    y => p23_tp6);


  u27 : g2 port map (
    a => t10,
    b => f,
    y => p22_tp5);


  u28 : g2 port map (
    a => h,
    b => c,
    y => w);


  u29 : g3 port map (
    a => u,
    b => w,
    c => j,
    y => t11);


  u30 : g3 port map (
    a => s,
    b => t11,
    c => v,
    y => p6_tp1);



end gates;

