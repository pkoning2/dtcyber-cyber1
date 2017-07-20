-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- JN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jn is
    port (
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p25_tp4 : out logicsig;
      p26_p27 : out logicsig);

end jn;
architecture gates of jn is
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
  signal p : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g4 port map (
    a => l,
    b => j,
    c => h,
    d => p,
    y => p25_tp4);


  u2 : g3 port map (
    a => l,
    b => j,
    c => g,
    y => tp6,
    y2 => t2);


  u3 : g2 port map (
    a => p,
    b => t2,
    y => p26_p27);


  u4 : g2 port map (
    a => t2,
    b => e,
    y2 => m);


  u5 : g3 port map (
    a => k,
    b => i,
    c => h,
    y2 => t4);


  u6 : g2 port map (
    a => t4,
    b => p,
    y => p18);


  u7 : g5 port map (
    a => t4,
    b => a,
    c => p,
    d => c,
    e => e,
    y2 => p20);


  u8 : inv2 port map (
    a => p23,
    y2 => p);


  u9 : inv2 port map (
    a => p14,
    y => e,
    y2 => f);


  u10 : inv2 port map (
    a => p12,
    y => c,
    y2 => d);


  u11 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u12 : g3 port map (
    a => b,
    b => d,
    c => m,
    y => tp1,
    y2 => p6);


  u13 : g3 port map (
    a => a,
    b => d,
    c => m,
    y => tp2,
    y2 => p5);


  u14 : g3 port map (
    a => b,
    b => c,
    c => m,
    y2 => p4);


  u15 : g3 port map (
    a => a,
    b => c,
    c => m,
    y => p2,
    y2 => p1);


  u16 : g2 port map (
    a => f,
    b => p,
    y => p9);


  u17 : g2 port map (
    a => d,
    b => p,
    y => p10);


  u18 : g2 port map (
    a => b,
    b => p,
    y => p7);


  u19 : inv2 port map (
    a => p24,
    y => k,
    y2 => l);


  u20 : inv2 port map (
    a => p22,
    y => i,
    y2 => j);


  u21 : inv2 port map (
    a => p19,
    y => g,
    y2 => h);



end gates;

