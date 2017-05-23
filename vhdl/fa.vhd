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
-- FA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity fa is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p6 : out logicsig;
      p8_p13 : out logicsig;
      p10 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end fa;
architecture gates of fa is
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
  signal q1 : logicsig;
  signal q2 : logicsig;
  signal q3 : logicsig;
  signal q4 : logicsig;
  signal q5 : logicsig;
  signal q6 : logicsig;
  signal qb1 : logicsig;
  signal qb2 : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p11,
    y => a);

  p10 <= a;

  u2 : rsflop port map (
    r => a,
    s => p18,
    q => q1,
    qb => qb1);

  tp2 <= q1;

  u3 : rsflop port map (
    r => a,
    s => p20,
    q => q2,
    qb => qb2);

  tp4 <= q2;

  u4 : g2 port map (
    a => q1,
    b => q2,
    y => b);


  u5 : g2 port map (
    a => qb1,
    b => qb2,
    y => c);


  u6 : g2 port map (
    a => b,
    b => c,
    y => p6);


  u7 : rsflop port map (
    r => a,
    s => p5,
    q => q3,
    qb => h);

  tp1 <= q3;

  u8 : rsflop port map (
    r => a,
    s => p3,
    q => q4,
    qb => i);

  tp3 <= q4;

  u9 : g2 port map (
    a => q3,
    b => q4,
    y => d);


  u10 : g2 port map (
    a => h,
    b => i,
    y => e);

  p2 <= e;

  u11 : g2 port map (
    a => d,
    b => e,
    y => p1);


  u12 : rsflop port map (
    r => a,
    s => p23,
    q => q5,
    qb => j);

  tp6 <= q5;

  u13 : rsflop port map (
    r => a,
    s => p26,
    q => q6,
    qb => k);


  u14 : g2 port map (
    a => q5,
    b => q6,
    y => f);

  p27 <= f;

  u15 : g2 port map (
    a => j,
    b => k,
    y => g);

  p25 <= g;

  u16 : g2 port map (
    a => f,
    b => g,
    y => p28);


  u17 : g3 port map (
    a => b,
    b => d,
    c => f,
    y => p15,
    y2 => p8_p13);


  u18 : g3 port map (
    a => h,
    b => i,
    c => b,
    y => t1);


  u19 : g4 port map (
    a => b,
    b => d,
    c => k,
    d => j,
    y => t2);


  u20 : g3 port map (
    a => c,
    b => t1,
    c => t2,
    y => p17);



end gates;

