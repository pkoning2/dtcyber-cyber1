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
-- CM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cm is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end cm;
architecture gates of cm is
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
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p6,
    b => g,
    y => t1);


  u2 : g2 port map (
    a => p5,
    b => g,
    y => t2);


  u3 : g2 port map (
    a => p4,
    b => g,
    y => t3);


  u4 : g2 port map (
    a => p2,
    b => g,
    y => t4);


  u5 : g5 port map (
    a => t1,
    b => t2,
    c => l,
    d => t3,
    e => t4,
    y => t5);

  p3 <= t5;
  p13 <= l;
  tp1 <= t5;

  u6 : g2 port map (
    a => p27,
    b => g,
    y2 => p28);


  u7 : g2 port map (
    a => p26,
    b => g,
    y2 => p25);


  u8 : g2 port map (
    a => p23,
    b => g,
    y2 => p24);


  u9 : g2 port map (
    a => p22,
    b => g,
    y2 => p21);


  u10 : inv port map (
    a => p20,
    y => j);


  u11 : inv2 port map (
    a => p18,
    y => k,
    y2 => t6);


  u12 : g4 port map (
    a => d,
    b => j,
    c => k,
    d => b,
    y => f,
    y2 => g);

  p19 <= f;
  tp6 <= f;

  u13 : g2 port map (
    a => j,
    b => t6,
    y => p16,
    y2 => e);


  u14 : inv2 port map (
    a => p15,
    y => c,
    y2 => d);


  u15 : inv2 port map (
    a => p17,
    y => a,
    y2 => b);


  u16 : g3 port map (
    a => e,
    b => b,
    c => d,
    y => tp3,
    y2 => p8);


  u17 : g3 port map (
    a => e,
    b => a,
    c => d,
    y => tp2,
    y2 => p7);


  u18 : g3 port map (
    a => e,
    b => c,
    c => b,
    y => tp4,
    y2 => p10);


  u19 : g3 port map (
    a => e,
    b => c,
    c => a,
    y2 => p9);


  u20 : g4 port map (
    a => j,
    b => k,
    c => d,
    d => a,
    y => l,
    y2 => t7);

  p11 <= t7;

  u21 : g2 port map (
    a => t7,
    b => p12,
    y => p14);



end gates;

