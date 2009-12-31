-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- HH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hh is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p20 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end hh;
architecture gates of hh is
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

  signal a : logicsig;
  signal b : logicsig;
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
  signal t : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p7,
    y => t1);

  tp3 <= t1;

  u2 : g3 port map (
    a => t,
    b => p8,
    c => t1,
    y => d);


  u3 : g3 port map (
    a => p6,
    b => t1,
    c => b,
    y => e);


  u4 : g3 port map (
    a => t1,
    b => p9,
    c => a,
    y => f);


  u5 : inv port map (
    a => p22,
    y => t2);

  tp4 <= t2;

  u6 : g3 port map (
    a => t,
    b => p21,
    c => t2,
    y => j);


  u7 : g3 port map (
    a => p23,
    b => t2,
    c => b,
    y => k);


  u8 : g3 port map (
    a => t2,
    b => p17,
    c => a,
    y => l);


  u9 : inv port map (
    a => p4,
    y => t3);

  tp1 <= t3;

  u10 : g4 port map (
    a => p3,
    b => p13,
    c => t,
    d => t3,
    y => g);


  u11 : g3 port map (
    a => p5,
    b => t3,
    c => b,
    y => h);


  u12 : g3 port map (
    a => t3,
    b => p11,
    c => a,
    y => i);


  u13 : inv port map (
    a => p25,
    y => t4);

  tp6 <= t4;

  u14 : g3 port map (
    a => t,
    b => p26,
    c => t4,
    y => m);


  u15 : g3 port map (
    a => p27,
    b => t4,
    c => b,
    y => n);


  u16 : g3 port map (
    a => t4,
    b => p19,
    c => a,
    y => p);


  u17 : g3 port map (
    a => t,
    b => t4,
    c => p12,
    y => p20);


  u18 : g6 port map (
    a => d,
    b => e,
    c => f,
    d => g,
    e => h,
    f => i,
    y => tp2,
    y2 => p1);


  u19 : g6 port map (
    a => j,
    b => k,
    c => l,
    d => m,
    e => n,
    f => p,
    y => tp5,
    y2 => p28);


  u20 : inv port map (
    a => p14,
    y => a);


  u21 : inv port map (
    a => p16,
    y => b);


  u22 : g2 port map (
    a => p14,
    b => p15,
    y => t);


  u23 : inv port map (
    a => p15,
    y => p24);



end gates;

