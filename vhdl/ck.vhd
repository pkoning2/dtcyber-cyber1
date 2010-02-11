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
-- CK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ck is
    port (
      p2 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7_tp4 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p24_tp2 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ck;
architecture gates of ck is
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
  signal m : logicsig;
  signal n : logicsig;
  signal o : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : rsflop port map (
    r => b,
    s => p2,
    q => d,
    qb => e);

  tp1 <= d;

  u2 : g2 port map (
    a => p2,
    b => e,
    y => j);


  u3 : inv port map (
    a => d,
    y => k);


  u4 : g2 port map (
    a => c,
    b => d,
    y => p5);

  tp3 <= c;

  u5 : rsflop port map (
    r => b,
    s => p8,
    q => f,
    qb => g);

  tp6 <= f;

  u6 : g2 port map (
    a => p8,
    b => g,
    y => l);


  u7 : inv port map (
    a => f,
    y => m);


  u8 : g2 port map (
    a => c,
    b => f,
    y => p19);


  u9 : rsflop port map (
    r => b,
    s => p25,
    q => h,
    qb => i);

  tp5 <= h;

  u10 : g2 port map (
    a => p25,
    b => i,
    y => n);


  u11 : inv port map (
    a => h,
    y => o);


  u12 : g2 port map (
    a => c,
    b => h,
    y => p26);


  u13 : inv2 port map (
    a => a,
    y2 => t1);

  p7_tp4 <= a;

  u14 : rsflop port map (
    r => b,
    s => t1,
    q => r);

  p24_tp2 <= r;

  u15 : g2 port map (
    a => p23,
    b => p9,
    y => a);


  u16 : inv port map (
    a => p9,
    y => b);


  u17 : inv port map (
    a => p10,
    y => c);


  u18 : g4 port map (
    a => k,
    b => r,
    c => g,
    d => i,
    y => p17);


  u19 : g3 port map (
    a => j,
    b => g,
    c => o,
    y => p28);


  u20 : g3 port map (
    a => k,
    b => f,
    c => i,
    y => p15);


  u21 : g3 port map (
    a => j,
    b => f,
    c => o,
    y => p21);


  u22 : g3 port map (
    a => e,
    b => m,
    c => h,
    y => p3);


  u23 : g3 port map (
    a => d,
    b => m,
    c => n,
    y => p11);


  u24 : g3 port map (
    a => e,
    b => l,
    c => h,
    y => p1);


  u25 : g3 port map (
    a => d,
    b => l,
    c => n,
    y => p13);



end gates;

