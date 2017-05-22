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
-- NQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nq is
    port (
      p3 : in  logicsig;
      p7 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      tp4 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11_tp2 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p20_tp3 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end nq;
architecture gates of nq is
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
  signal n : logicsig;
  signal p : logicsig;
  signal q : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t10 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t15 : logicsig;
  signal v : logicsig;
  signal w : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p15,
    y => c,
    y2 => d);


  u3 : inv2 port map (
    a => p18,
    y => e,
    y2 => f);


  u4 : inv2 port map (
    a => p19,
    y => g,
    y2 => h);


  u5 : g2 port map (
    a => p7,
    b => a,
    y => p5);


  u6 : inv port map (
    a => p7,
    y => j);


  u7 : g2 port map (
    a => j,
    b => b,
    y => p2);


  u8 : g2 port map (
    a => j,
    b => a,
    y => p4);


  u9 : inv port map (
    a => j,
    y => t1);


  u10 : g2 port map (
    a => t1,
    b => k,
    y => t2);


  u11 : g3 port map (
    a => t2,
    b => q,
    c => r,
    y => t3,
    y2 => t4);

  tp4 <= r;

  u12 : g2 port map (
    a => t3,
    b => e,
    y => p21);


  u13 : g2 port map (
    a => t4,
    b => f,
    y => p24);


  u14 : inv port map (
    a => p3,
    y => p1);


  u15 : g2 port map (
    a => n,
    b => p,
    y => p25,
    y2 => p26);


  u16 : g2 port map (
    a => d,
    b => h,
    y => n);


  u17 : g2 port map (
    a => h,
    b => f,
    y => w);

  p28 <= w;

  u18 : g2 port map (
    a => n,
    b => w,
    y => p22);


  u19 : g2 port map (
    a => d,
    b => g,
    y => r);


  u20 : g2 port map (
    a => c,
    b => h,
    y => k,
    y2 => p12);


  u21 : g2 port map (
    a => k,
    b => b,
    y => q);


  u23 : g3 port map (
    a => r,
    b => q,
    c => e,
    y => p20_tp3);


  u24 : inv port map (
    a => p8,
    y => t10);


  u25 : g2 port map (
    a => a,
    b => t10,
    y => p11_tp2);


  u26 : g2 port map (
    a => p8,
    b => b,
    y => p6,
    y2 => t12);

  p9 <= t12;

  u27 : inv port map (
    a => t12,
    y => p10);


  u28 : g2 port map (
    a => c,
    b => g,
    y => p);


  u29 : g2 port map (
    a => t12,
    b => p,
    y => t13);


  u30 : g2 port map (
    a => t13,
    b => n,
    y => p14);


  u31 : g2 port map (
    a => e,
    b => g,
    y => t15);


  u32 : g2 port map (
    a => t15,
    b => w,
    y => v,
    y2 => p27);



end gates;

