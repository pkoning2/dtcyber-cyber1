-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- PV module, rev D
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pv is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : inout misc;
      p18 : inout misc;
      p20 : inout misc;
      p22 : inout misc;
      p24 : inout misc;
      p25 : in  logicsig;
      p26 : inout misc;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p7 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p28 : out logicsig);

end pv;
architecture gates of pv is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p27,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p25,
    y => c,
    y2 => d);


  u3 : inv2 port map (
    a => p14,
    y => e,
    y2 => f);


  u4 : inv2 port map (
    a => p12,
    y => g,
    y2 => h);


  u5 : inv2 port map (
    a => p5,
    y => i,
    y2 => j);


  u6 : inv2 port map (
    a => p10,
    y => k,
    y2 => l);


  u7 : g2 port map (
    a => l,
    b => p6,
    y => t1);


  u8 : g4 port map (
    a => k,
    b => j,
    c => g,
    d => e,
    y => t2);


  u9 : g2 port map (
    a => h,
    b => j,
    y => t3);


  u10 : g5 port map (
    a => t1,
    b => p2,
    c => p4,
    d => t2,
    e => t3,
    y => t4);

  tp1 <= t4;

  u11 : g2 port map (
    a => p3,
    b => t4,
    y => p1);


  u12 : g4 port map (
    a => h,
    b => p8,
    c => i,
    d => k,
    y2 => m);


  u13 : g4 port map (
    a => m,
    b => a,
    c => c,
    d => e,
    y => tp2,
    y2 => p13);


  u14 : g3 port map (
    a => m,
    b => e,
    c => b,
    y => t5);


  u15 : g3 port map (
    a => m,
    b => e,
    c => d,
    y => t6);

  p17 <= t6;
  tp6 <= t6;

  u16 : g2 port map (
    a => t5,
    b => t6,
    y => tp5,
    y2 => p28);


  u17 : g3 port map (
    a => m,
    b => f,
    c => c,
    y => p7);


  u18 : g2 port map (
    a => m,
    b => a,
    y => p11);


  u19 : g2 port map (
    a => m,
    b => b,
    y => p19);



end gates;

