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
-- NW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nw is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p14_tp2 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig);

end nw;
architecture gates of nw is
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
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal p : logicsig;
  signal q : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => e,
    y => k);

  p7 <= k;

  u2 : g2 port map (
    a => b,
    b => f,
    y => l);

  p4 <= f;

  u3 : g2 port map (
    a => k,
    b => l,
    y => t1,
    y2 => n);

  p14_tp2 <= t1;

  u4 : inv port map (
    a => p2,
    y => t2);


  u5 : g3 port map (
    a => p9,
    b => t2,
    c => p11,
    y => t3,
    y2 => t4);

  tp1 <= t3;

  u6 : g3 port map (
    a => q,
    b => t1,
    c => t4,
    y => p3);

  p5 <= q;

  u7 : g3 port map (
    a => n,
    b => t3,
    c => q,
    y => p6);


  u8 : g2 port map (
    a => t3,
    b => l,
    y => t5);


  u9 : g2 port map (
    a => t5,
    b => k,
    y => t6,
    y2 => t7);

  tp6 <= t6;

  u10 : g3 port map (
    a => p,
    b => t6,
    c => m,
    y => p17);


  u11 : g3 port map (
    a => m,
    b => t6,
    c => q,
    y => p19);


  u12 : g2 port map (
    a => c,
    b => g,
    y => i);

  p26 <= g;

  u13 : g2 port map (
    a => d,
    b => h,
    y => j);

  p23 <= h;

  u14 : g2 port map (
    a => i,
    b => j,
    y => t8,
    y2 => m);

  tp5 <= t8;

  u15 : g3 port map (
    a => q,
    b => t7,
    c => t8,
    y => p20);


  u16 : g3 port map (
    a => t7,
    b => t8,
    c => p,
    y => p16);


  u17 : g2 port map (
    a => p12,
    b => p10,
    y => a,
    y2 => b);


  u18 : g2 port map (
    a => p27,
    b => p25,
    y => c,
    y2 => d);


  u19 : inv2 port map (
    a => p8,
    y => e,
    y2 => f);


  u20 : inv2 port map (
    a => p28,
    y => g,
    y2 => h);


  u21 : inv port map (
    a => p13,
    y => p);


  u23 : g3 port map (
    a => j,
    b => a,
    c => e,
    y => t10);


  u24 : g2 port map (
    a => t10,
    b => i,
    y => p18,
    y2 => p15);


  u25 : g2 port map (
    a => j,
    b => l,
    y => p24);


  u26 : inv port map (
    a => p1,
    y => q);



end gates;

