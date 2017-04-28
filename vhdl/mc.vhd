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
-- MC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mc is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p2 : out logicsig;
      p4_tp2 : out logicsig;
      p5_tp1 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p18 : out logicsig;
      p20_tp5 : out logicsig;
      p24_tp6 : out logicsig;
      p28 : out logicsig);

end mc;
architecture gates of mc is
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
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal m : logicsig;
  signal n : logicsig;
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

begin -- gates
  u1 : g2 port map (
    a => t1,
    b => t3,
    y => p5_tp1);


  u2 : g3 port map (
    a => p3,
    b => p6,
    c => t4,
    y => t3);

  p4_tp2 <= t4;

  u3 : g2 port map (
    a => p3,
    b => p6,
    y => t2);

  p2 <= t2;

  u4 : g3 port map (
    a => t2,
    b => i,
    c => j,
    y => t1);


  u5 : g2 port map (
    a => i,
    b => j,
    y => t4);


  u6 : inv2 port map (
    a => p1,
    y => k,
    y2 => p11);


  u7 : g2 port map (
    a => m,
    b => n,
    y => p18);


  u8 : g2 port map (
    a => t11,
    b => k,
    y => i);

  p7 <= t11;

  u9 : g3 port map (
    a => h,
    b => t5,
    c => p1,
    y => j);


  u10 : g2 port map (
    a => h,
    b => t5,
    y => t11);


  u11 : g3 port map (
    a => n,
    b => e,
    c => f,
    y => t5);


  u12 : g2 port map (
    a => p8,
    b => p10,
    y => n);


  u13 : g3 port map (
    a => p8,
    b => p10,
    c => m,
    y => h);


  u14 : g2 port map (
    a => e,
    b => f,
    y => m);


  u15 : g2 port map (
    a => m,
    b => n,
    y => p9);


  u16 : inv2 port map (
    a => p23,
    y => g,
    y2 => t6);


  u17 : g2 port map (
    a => p23,
    b => t8,
    y => p20_tp5);


  u18 : g2 port map (
    a => g,
    b => t8,
    y => e);


  u19 : g2 port map (
    a => t7,
    b => d,
    y => t8);


  u20 : g3 port map (
    a => d,
    b => t7,
    c => t6,
    y => f);


  u23 : g3 port map (
    a => b,
    b => a,
    c => t12,
    y => t7);


  u24 : g3 port map (
    a => p26,
    b => p22,
    c => t13,
    y => d);


  u25 : g2 port map (
    a => p26,
    b => p22,
    y => t12);


  u26 : g2 port map (
    a => a,
    b => b,
    y => t13);


  u27 : g2 port map (
    a => t12,
    b => t13,
    y => p24_tp6);


  u28 : inv2 port map (
    a => p27,
    y => c,
    y2 => t9);


  u29 : g2 port map (
    a => c,
    b => t10,
    y => a);


  u30 : g2 port map (
    a => p27,
    b => t10,
    y => p28);


  u31 : inv port map (
    a => p25,
    y => t10);


  u32 : g2 port map (
    a => t9,
    b => p25,
    y => b);



end gates;

