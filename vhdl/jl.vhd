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
-- JL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jl is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p28 : out logicsig);

end jl;
architecture gates of jl is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal m : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p2,
    b => p4,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => a,
    y => h);

  p14 <= h;

  u3 : rsflop port map (
    r => c,
    s => h,
    q => t2);

  tp2 <= t2;

  u4 : g2 port map (
    a => d,
    b => t2,
    y => f,
    y2 => x);

  p10 <= x;
  tp1 <= d;

  u5 : inv port map (
    a => t2,
    y => e);

  p5 <= e;

  u6 : inv port map (
    a => p22,
    y => t3);


  u7 : g2 port map (
    a => p23,
    b => p21,
    y => t4);


  u8 : g2 port map (
    a => t3,
    b => a,
    y => t5);


  u9 : g2 port map (
    a => t4,
    b => a,
    y => t6);


  u10 : rs2flop port map (
    r => b,
    s => t5,
    s2 => t6,
    q => m,
    qb => r);

  p3 <= r;
  tp4 <= m;
  tp5 <= b;

  u11 : inv port map (
    a => m,
    y => p19);


  u12 : g4 port map (
    a => e,
    b => h,
    c => m,
    d => d,
    y => g,
    y2 => p18);


  u13 : g2 port map (
    a => g,
    b => f,
    y => p13);


  u14 : g2 port map (
    a => t4,
    b => p11,
    y => t8);


  u15 : rsflop port map (
    r => b,
    s => t8,
    q => t9);

  tp6 <= t9;

  u16 : inv port map (
    a => t9,
    y => p20);


  u17 : g2 port map (
    a => r,
    b => e,
    y => t10);


  u18 : g2 port map (
    a => t10,
    b => p9,
    y => t11);


  u19 : g2 port map (
    a => p12,
    b => d,
    y => t12);

  u20 : rsflop port map (
    r => t12,
    s => t11,
    q => t13);

  tp3 <= t13;
  p7 <= t13;


  u21 : inv2 port map (
    a => p15,
    y2 => a);


  u22 : inv port map (
    a => p17,
    y => b);


  u23 : inv port map (
    a => p16,
    y => c);


  u24 : g2 port map (
    a => p6,
    b => p8,
    y => d);


  u25 : g4 port map (
    a => p24,
    b => p25,
    c => p26,
    d => p27,
    y => p28);


end gates;

