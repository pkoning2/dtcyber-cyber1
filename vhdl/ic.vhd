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
-- IC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ic is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ic;
architecture gates of ic is
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

begin -- gates
  u1 : rsflop port map (
    r => b,
    s => p4,
    q => d,
    qb => e);

  tp1 <= d;

  u2 : g2 port map (
    a => p4,
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
    s => p6,
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


  u16 : inv port map (
    a => p9,
    y => b);

  p7 <= b;
  
  u17 : inv port map (
    a => p8,
    y => c);


  u18 : g4 port map (
    a => k,
    b => p23,
    c => g,
    d => i,
    y => p15);


  u19 : g3 port map (
    a => j,
    b => g,
    c => o,
    y => p28);


  u20 : g3 port map (
    a => k,
    b => f,
    c => i,
    y => p17);


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

