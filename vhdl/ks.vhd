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
-- KS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ksslice is
    port (
      a : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      q : out logicsig);

end ksslice;
architecture gates of ksslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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
  signal t3 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => f,
    b => a,
    y => t1);


  u2 : inv port map (
    a => a,
    y => t2);


  u3 : g2 port map (
    a => t2,
    b => e,
    y => t3);


  u4 : g2 port map (
    a => t1,
    b => t3,
    y => q);



end gates;

use work.sigs.all;

entity ks is
    port (
      p1 : in  logicsig;
      p4 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      tp4 : out logicsig;
      p2_tp3 : out logicsig;
      p3_tp1 : out logicsig;
      p5_p9_tp2 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p11_p20 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p23_p24_tp5 : out logicsig;
      p26_tp6 : out logicsig;
      p27 : out logicsig);

end ks;
architecture gates of ks is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component ksslice
    port (
      a : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      q : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : ksslice port map (
    a => p28,
    e => e,
    f => f,
    q => a);

  p8 <= f;
  p27 <= a;

  u2 : ksslice port map (
    a => p25,
    e => e,
    f => f,
    q => b);

  p26_tp6 <= b;

  u3 : ksslice port map (
    a => p1,
    e => e,
    f => f,
    q => c);

  p2_tp3 <= c;

  u4 : ksslice port map (
    a => p4,
    e => e,
    f => f,
    q => d);

  p3_tp1 <= d;

  u5 : ksslice port map (
    a => p10,
    e => e,
    f => f,
    q => t1);

  p5_p9_tp2 <= t1;

  u6 : inv port map (
    a => t1,
    y => p6);


  u7 : ksslice port map (
    a => p22,
    e => e,
    f => f,
    q => p23_p24_tp5);


  u8 : g6 port map (
    a => p18,
    b => a,
    c => b,
    d => c,
    e => d,
    f => p12,
    y => tp4,
    y2 => p11_p20);


  u9 : g2 port map (
    a => p21,
    b => p19,
    y => p15);


  u10 : g2 port map (
    a => p14,
    b => p19,
    y => p16,
    y2 => p13);


  u11 : inv2 port map (
    a => p7,
    y => e,
    y2 => f);



end gates;

