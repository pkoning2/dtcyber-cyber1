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
-- KC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kc is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p3_tp3 : out logicsig;
      p6 : out logicsig;
      p8_tp3 : out logicsig;
      p9_tp2 : out logicsig;
      p14_tp5 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18_tp4 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p24_tp6 : out logicsig);

end kc;
architecture gates of kc is
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

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal o : logicsig;
  signal p : logicsig;
  signal q : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t : logicsig;
  signal u : logicsig;
  signal v : logicsig;
  signal w : logicsig;
  signal y : logicsig;

begin -- gates
  u1 : inv port map (
    a => p2,
    y => b);


  u2 : g4 port map (
    a => m,
    b => n,
    c => o,
    d => p,
    y2 => q);

  p19 <= p;

  u4 : g5 port map (
    a => p4,
    b => k,
    c => q,
    d => l,
    e => p2,
    y => p16,
    y2 => p3_tp3);


  u5 : inv2 port map (
    a => p12,
    y => c,
    y2 => k);


  u6 : inv2 port map (
    a => p11,
    y => d,
    y2 => l);


  u7 : inv2 port map (
    a => p27,
    y => e,
    y2 => m);


  u8 : inv2 port map (
    a => p28,
    y => f,
    y2 => n);


  u9 : inv2 port map (
    a => p21,
    y => g,
    y2 => o);


  u10 : inv port map (
    a => p13,
    y => y);


  u11 : inv2 port map (
    a => p25,
    y => h,
    y2 => p);


  u12 : g2 port map (
    a => g,
    b => p,
    y => r);

  p20 <= r;

  u13 : g3 port map (
    a => f,
    b => o,
    c => p,
    y => s);

  p17 <= s;

  u14 : g4 port map (
    a => e,
    b => n,
    c => o,
    d => p,
    y => t);

  p24_tp6 <= t;

  u15 : g2 port map (
    a => d,
    b => q,
    y => u);

  p6 <= u;

  u16 : g3 port map (
    a => c,
    b => l,
    c => q,
    y => p9_tp2);


  u17 : g4 port map (
    a => b,
    b => k,
    c => l,
    d => q,
    y => w);

  p1 <= w;

  u18 : g5 port map (
    a => p,
    b => r,
    c => s,
    d => t,
    e => y,
    y => p8_tp3);


  u19 : g5 port map (
    a => p,
    b => r,
    c => u,
    d => v,
    e => y,
    y => p18_tp4);


  u20 : g6 port map (
    a => p,
    b => s,
    c => p23,
    d => w,
    e => y,
    f => u,
    y => p14_tp5);



end gates;

