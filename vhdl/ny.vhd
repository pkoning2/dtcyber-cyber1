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
-- NY module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nyslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      x : out logicsig;
      y : out logicsig);

end nyslice;
architecture gates of nyslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => i2,
    y => t1);


  u2 : inv port map (
    a => i2,
    y => t2);


  u3 : g2 port map (
    a => t2,
    b => i3,
    y => t4);


  u4 : g2 port map (
    a => t1,
    b => t4,
    y => t5);


  u5 : g2 port map (
    a => t5,
    b => b,
    y => x);


  u6 : g3 port map (
    a => t1,
    b => t4,
    c => a,
    y => y);



end gates;

use work.sigs.all;

entity ny is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p10_p12_p14 : out logicsig;
      p26 : out logicsig;
      p28_tp6 : out logicsig);

end ny;
architecture gates of ny is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component nyslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      x : out logicsig;
      y : out logicsig);

  end component;

  signal c : logicsig;
  signal n : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal u : logicsig;
  signal v : logicsig;
  signal w : logicsig;
  signal x : logicsig;
  signal y : logicsig;
  signal z : logicsig;

begin -- gates
  u1 : nyslice port map (
    a => n,
    b => c,
    i1 => p27,
    i2 => p18,
    i3 => p24,
    x => t1,
    y => t2);

  tp3 <= n;

  u2 : g2 port map (
    a => t1,
    b => t2,
    y => p26,
    y2 => p28_tp6);


  u3 : inv port map (
    a => p1,
    y => t3);


  u4 : nyslice port map (
    a => n,
    b => c,
    i1 => p2,
    i2 => t3,
    i3 => p4,
    x => u,
    y => v);

  tp2 <= v;

  u5 : inv port map (
    a => p6,
    y => t4);


  u7 : nyslice port map (
    a => n,
    b => c,
    i1 => p5,
    i2 => t4,
    i3 => p3,
    x => w,
    y => x);

  tp1 <= x;

  u8 : inv port map (
    a => p16,
    y => t5);


  u9 : nyslice port map (
    a => n,
    b => c,
    i1 => p21,
    i2 => t5,
    i3 => p23,
    x => y,
    y => z);


  u10 : g2 port map (
    a => p7,
    b => p19,
    y => n);


  u11 : g2 port map (
    a => p19,
    b => n,
    y => c);


  u12 : g6 port map (
    a => u,
    b => v,
    c => w,
    d => x,
    e => y,
    f => z,
    y => tp4,
    y2 => p10_p12_p14);



end gates;

