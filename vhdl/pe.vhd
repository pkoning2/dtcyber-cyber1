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
-- PE module rev D
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pe is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p13_p14 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pe;
architecture gates of pe is
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

  signal a : logicsig;
  signal b : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal i : logicsig;
  signal p : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t : logicsig;
  signal u : logicsig;
  signal v : logicsig;
  signal w : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p4,
    y => i,
    y2 => a);


  u2 : inv2 port map (
    a => p6,
    y => e,
    y2 => f);


  u3 : inv2 port map (
    a => p25,
    y2 => b);


  u4 : inv port map (
    a => p26,
    y => d);


  u5 : g3 port map (
    a => b,
    b => p10,
    c => a,
    y => x,
    y2 => p9);


  u6 : g3 port map (
    a => b,
    b => p21,
    c => f,
    y => p,
    y2 => p15);


  u7 : g3 port map (
    a => b,
    b => p3,
    c => i,
    y => u,
    y2 => p11);


  u8 : g4 port map (
    a => b,
    b => p8,
    c => e,
    d => a,
    y => s);


  u9 : g6 port map (
    a => p,
    b => s,
    c => u,
    d => v,
    e => w,
    f => x,
    y => tp1,
    y2 => p17);

  tp3 <= v;
  tp4 <= w;

  u10 : g5 port map (
    a => b,
    b => p23,
    c => d,
    d => e,
    e => a,
    y => t);

  tp2 <= t;

  u11 : g3 port map (
    a => b,
    b => p22,
    c => i,
    y => v);


  u12 : g3 port map (
    a => b,
    b => p5,
    c => i,
    y => w);


  u13 : g4 port map (
    a => b,
    b => p24,
    c => d,
    d => f,
    y => r);

  tp5 <= r;

  u14 : inv2 port map (
    a => r,
    y2 => p1_p13_p14);


  u15 : g3 port map (
    a => r,
    b => t,
    c => w,
    y => tp6,
    y2 => p28);


  u16 : g2 port map (
    a => r,
    b => t,
    y => p27,
    y2 => p20);


  u17 : g3 port map (
    a => b,
    b => d,
    c => i,
    y => p18);


  u18 : g2 port map (
    a => p,
    b => r,
    y => p19);


  u19 : g2 port map (
    a => v,
    b => w,
    y => p12);



end gates;

