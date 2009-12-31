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
-- AG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ag is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p9 : out logicsig;
      p15 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ag;
architecture gates of ag is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
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

  signal c : logicsig;
  signal g : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t : logicsig;
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
  signal t12 : logicsig;
  signal t13 : logicsig;

begin -- gates
  u1 : g5 port map (
    a => p21,
    b => p8,
    c => c,
    d => g,
    e => t,
    y => t1);


  u2 : r2sflop port map (
    r => r,
    r2 => s,
    s => t1,
    q => t2);

  p15 <= s;
  tp4 <= t2;

  u3 : g3 port map (
    a => t2,
    b => j,
    c => p3,
    y => k);

  tp1 <= j;

  u4 : inv port map (
    a => t2,
    y => t3);


  u5 : g5 port map (
    a => p13,
    b => p10,
    c => c,
    d => g,
    e => t,
    y => t4);


  u6 : r2sflop port map (
    r => r,
    r2 => s,
    s => t4,
    q => t5);


  u7 : g3 port map (
    a => p3,
    b => j,
    c => t5,
    y => l);


  u8 : inv port map (
    a => t5,
    y => t6);


  u9 : g2 port map (
    a => t3,
    b => t6,
    y => t7);


  u10 : g4 port map (
    a => p7,
    b => p11,
    c => t7,
    d => p6,
    y => t8);

  p9 <= t8;

  u11 : rsflop port map (
    r => r,
    s => t8,
    q => j);


  u12 : inv port map (
    a => j,
    y => p5);


  u13 : rsflop port map (
    r => r,
    s => k,
    q => t9);

  p2 <= t9;
  tp2 <= t9;

  u14 : rsflop port map (
    r => r,
    s => l,
    q => t10);

  p26 <= t10;
  tp6 <= t10;

  u15 : inv port map (
    a => p19,
    y => c);


  u16 : inv port map (
    a => p17,
    y => r);


  u17 : g6 port map (
    a => p23,
    b => p18,
    c => p25,
    d => p20,
    e => p22,
    f => p24,
    y2 => g);


  u18 : g2 port map (
    a => k,
    b => l,
    y => p28,
    y2 => p27);


  u19 : inv2 port map (
    a => j,
    y2 => p4);


  u20 : inv port map (
    a => p12,
    y => s);


  u21 : inv port map (
    a => p1,
    y => t12);


  u22 : inv2 port map (
    a => t12,
    y2 => t13);


  u23 : g2 port map (
    a => p12,
    b => t13,
    y => tp3,
    y2 => t);



end gates;

