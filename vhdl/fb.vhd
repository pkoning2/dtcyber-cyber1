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
-- FB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity fb is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6_tp2 : out logicsig;
      p7_p8_p9_p10_p12 : out logicsig;
      p17_p20 : out logicsig;
      p21_p22 : out logicsig;
      p23_p24 : out logicsig;
      p26_p28 : out logicsig);

end fb;
architecture gates of fb is
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
  signal t1 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : g4 port map (
    a => e,
    b => f,
    c => g,
    d => h,
    y => tp1,
    y2 => t1);

  tp3 <= h;

  u2 : inv port map (
    a => t1,
    y => p7_p8_p9_p10_p12);


  u3 : inv port map (
    a => p4,
    y => a);


  u4 : inv port map (
    a => p3,
    y => b);


  u5 : inv port map (
    a => p1,
    y => c);


  u6 : inv port map (
    a => p2,
    y => t3);


  u7 : g4 port map (
    a => a,
    b => b,
    c => c,
    d => t3,
    y => p6_tp2);


  u8 : inv port map (
    a => p5,
    y => e);


  u9 : inv port map (
    a => p15,
    y => i);


  u10 : g2 port map (
    a => p15,
    b => a,
    y => f);


  u11 : inv2 port map (
    a => p16,
    y => j,
    y2 => t5);


  u12 : g2 port map (
    a => b,
    b => t5,
    y => l);

  tp4 <= l;

  u13 : g3 port map (
    a => p16,
    b => a,
    c => b,
    y => g);


  u14 : inv2 port map (
    a => p13,
    y => k,
    y2 => t6);


  u15 : g2 port map (
    a => c,
    b => t6,
    y => m);

  tp5 <= m;

  u16 : g3 port map (
    a => t6,
    b => c,
    c => b,
    y => n);

  tp6 <= n;

  u17 : g4 port map (
    a => a,
    b => p13,
    c => c,
    d => b,
    y => h);


  u18 : inv port map (
    a => p27,
    y => p26_p28);


  u19 : g2 port map (
    a => p25,
    b => k,
    y => p23_p24);


  u20 : g3 port map (
    a => p19,
    b => j,
    c => m,
    y => p21_p22);


  u21 : g4 port map (
    a => p18,
    b => i,
    c => l,
    d => n,
    y => p17_p20);



end gates;

