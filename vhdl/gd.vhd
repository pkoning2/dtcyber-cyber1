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
-- GD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gd is
    port (
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      tp1 : out logicsig;
      p1_p4 : out logicsig;
      p5_tp2 : out logicsig;
      p12_p21_tp3 : out logicsig;
      p15_tp4 : out logicsig;
      p19 : out logicsig;
      p24_tp5 : out logicsig;
      p26_tp6 : out logicsig;
      p27_p28 : out logicsig);

end gd;
architecture gates of gd is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p18,
    b => p10,
    y => j);


  u2 : g3 port map (
    a => p10,
    b => p18,
    c => i,
    y => m);

  tp1 <= i;

  u3 : inv port map (
    a => p20,
    y => n);


  u4 : inv2 port map (
    a => p9,
    y => t1,
    y2 => k);


  u5 : inv port map (
    a => p9,
    y => i);


  u6 : g2 port map (
    a => t1,
    b => i,
    y => l);


  u7 : g3 port map (
    a => l,
    b => m,
    c => n,
    y => t2);


  u8 : g2 port map (
    a => j,
    b => k,
    y => t3);


  u9 : g2 port map (
    a => p8,
    b => t3,
    y => e);


  u10 : g3 port map (
    a => e,
    b => f,
    c => a,
    y => p5_tp2);


  u11 : g3 port map (
    a => i,
    b => k,
    c => j,
    y => f);


  u12 : g2 port map (
    a => t2,
    b => p22,
    y => g);


  u13 : inv port map (
    a => p22,
    y => t5);

  p24_tp5 <= t5;

  u14 : g4 port map (
    a => t5,
    b => n,
    c => m,
    d => l,
    y => h);


  u15 : g3 port map (
    a => g,
    b => h,
    c => a,
    y => p26_tp6);


  u16 : inv port map (
    a => p16,
    y => a);


  u17 : g2 port map (
    a => p13,
    b => p17,
    y => c);


  u18 : inv port map (
    a => p17,
    y => t7);

  p15_tp4 <= t7;

  u19 : inv port map (
    a => p13,
    y => t8);


  u20 : g2 port map (
    a => t7,
    b => t8,
    y => d);


  u21 : g3 port map (
    a => a,
    b => c,
    c => d,
    y => p19);


  u22 : g3 port map (
    a => a,
    b => e,
    c => f,
    y => p1_p4);


  u23 : g3 port map (
    a => a,
    b => g,
    c => h,
    y => p27_p28);


  u24 : g3 port map (
    a => a,
    b => c,
    c => d,
    y => p12_p21_tp3);



end gates;

