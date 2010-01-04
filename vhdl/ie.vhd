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
-- IE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ie is
    port (
      p9 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
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
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p28 : out logicsig);

end ie;
architecture gates of ie is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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
  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p16,
    y => a);


  u2 : rsflop port map (
    r => a,
    s => p9,
    q => e);

  tp1 <= e;

  u3 : inv port map (
    a => e,
    y => f);


  u4 : rsflop port map (
    r => a,
    s => p13,
    q => c);

  tp2 <= c;

  u5 : inv port map (
    a => c,
    y => d);


  u6 : rsflop port map (
    r => a,
    s => p10,
    q => g);

  tp3 <= g;

  u7 : inv port map (
    a => g,
    y => h);


  u8 : rsflop port map (
    r => a,
    s => p14,
    q => i);

  tp4 <= i;

  u9 : inv port map (
    a => i,
    y => j);


  u10 : g2 port map (
    a => h,
    b => j,
    y => p2);


  u11 : g2 port map (
    a => d,
    b => f,
    y => p1);


  u12 : g2 port map (
    a => g,
    b => j,
    y => p5);


  u13 : g2 port map (
    a => c,
    b => f,
    y => p6);


  u14 : g2 port map (
    a => h,
    b => i,
    y => p7);


  u15 : g2 port map (
    a => e,
    b => d,
    y => p3);


  u16 : g2 port map (
    a => g,
    b => i,
    y => p17);


  u17 : g2 port map (
    a => c,
    b => e,
    y => p4);


  u18 : g4 port map (
    a => p24,
    b => p22,
    c => p20,
    d => p18,
    y => tp5,
    y2 => p19);


  u19 : g4 port map (
    a => p27,
    b => p25,
    c => p23,
    d => p21,
    y => t1);

  tp6 <= t1;

  u20 : g2 port map (
    a => p26,
    b => t1,
    y => p28);



end gates;

