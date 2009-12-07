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
-- MA module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ma is
    port (
      p2 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end ma;
architecture gates of ma is
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
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal n : logicsig;
  signal o : logicsig;
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
  u1 : rsflop port map (
    r => n,
    s => p11,
    q => a,
    qb => b);

  p6 <= n;
  p14 <= n;
  tp1 <= a;

  u2 : inv port map (
    a => a,
    y => c);

  p12 <= c;
  p28 <= c;

  u3 : inv port map (
    a => b,
    y => d);

  p3 <= d;

  u4 : rsflop port map (
    r => n,
    s => p10,
    q => e,
    qb => f);

  tp5 <= e;

  u5 : inv port map (
    a => e,
    y => g);

  p9 <= g;
  p23 <= g;

  u6 : inv port map (
    a => f,
    y => h);

  p7 <= h;

  u7 : rsflop port map (
    r => n,
    s => p15,
    q => i,
    qb => j);

  p16 <= j;
  tp6 <= i;

  u8 : inv port map (
    a => i,
    y => k);

  p18 <= k;

  u9 : inv port map (
    a => j,
    y => l);

  p17 <= l;

  u10 : inv port map (
    a => p2,
    y => o);


  u11 : g2 port map (
    a => p2,
    b => d,
    y => t1);


  u12 : g2 port map (
    a => c,
    b => o,
    y => t2);


  u13 : g2 port map (
    a => t1,
    b => t2,
    y => p1);


  u14 : g2 port map (
    a => d,
    b => h,
    y => t3);


  u15 : g2 port map (
    a => g,
    b => b,
    y => t4);


  u16 : g2 port map (
    a => t3,
    b => t4,
    y => p5);


  u17 : g2 port map (
    a => h,
    b => i,
    y => t5);


  u18 : g2 port map (
    a => k,
    b => f,
    y => t6);


  u19 : g2 port map (
    a => t5,
    b => t6,
    y => p8);


  u20 : inv port map (
    a => k,
    y => p19);


  u21 : inv port map (
    a => p13,
    y => n);


  u22 : g2 port map (
    a => l,
    b => h,
    y => t7);


  u23 : g2 port map (
    a => d,
    b => p27,
    y => t8,
    y2 => t9);


  u24 : g2 port map (
    a => l,
    b => t9,
    y => t10);


  u25 : inv port map (
    a => p27,
    y => t11);


  u26 : g2 port map (
    a => b,
    b => t11,
    y => t12);


  u27 : g2 port map (
    a => t12,
    b => h,
    y => t13);


  u28 : g3 port map (
    a => t7,
    b => t10,
    c => t13,
    y => p21);


  u29 : g2 port map (
    a => t13,
    b => t8,
    y => p25);



end gates;

