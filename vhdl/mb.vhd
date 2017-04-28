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
-- MB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mbslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig);

end mbslice;
architecture gates of mbslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);


  u2 : g2 port map (
    a => c,
    b => d,
    y => t2);


  u3 : g3 port map (
    a => t1,
    b => e,
    c => t2,
    y => y);



end gates;

use work.sigs.all;

entity mbslice2 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      y : out logicsig);

end mbslice2;
architecture gates of mbslice2 is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);


  u2 : g2 port map (
    a => c,
    b => d,
    y => t2);


  u3 : g2 port map (
    a => e,
    b => f,
    y => t3);


  u4 : g4 port map (
    a => t1,
    b => t2,
    c => t3,
    d => g,
    y => y);



end gates;

use work.sigs.all;

entity mb is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p6_tp3 : out logicsig;
      p7 : out logicsig;
      p8_tp2 : out logicsig;
      p13_tp1 : out logicsig;
      p14 : out logicsig;
      p19 : out logicsig;
      p20_tp5 : out logicsig;
      p22_tp4 : out logicsig;
      p26_tp6 : out logicsig);

end mb;
architecture gates of mb is
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

  component mbslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig);

  end component;

  component mbslice2
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;

begin -- gates
  u1 : inv port map (
    a => p25,
    y => a);


  u2 : inv port map (
    a => p28,
    y => b);


  u3 : inv port map (
    a => p16,
    y => c);


  u4 : inv port map (
    a => p10,
    y => d);


  u5 : inv port map (
    a => p5,
    y => e);


  u6 : inv2 port map (
    a => p12,
    y => f,
    y2 => p7);


  u7 : inv2 port map (
    a => p2,
    y => g,
    y2 => p1);


  u8 : inv2 port map (
    a => p17,
    y => h,
    y2 => p19);

  p14 <= h;

  u9 : mbslice2 port map (
    a => f,
    b => a,
    c => b,
    d => p9,
    e => c,
    f => p11,
    g => p18,
    y => p20_tp5);


  u10 : mbslice port map (
    a => p11,
    b => d,
    c => e,
    d => f,
    e => p15,
    y => p13_tp1);


  u11 : mbslice2 port map (
    a => g,
    b => a,
    c => b,
    d => p23,
    e => c,
    f => f,
    g => p21,
    y => p22_tp4);


  u12 : mbslice port map (
    a => f,
    b => d,
    c => e,
    d => g,
    e => p4,
    y => p8_tp2);


  u13 : mbslice2 port map (
    a => h,
    b => a,
    c => b,
    d => p27,
    e => c,
    f => g,
    g => p24,
    y => p26_tp6);


  u14 : mbslice port map (
    a => g,
    b => d,
    c => e,
    d => h,
    e => p3,
    y => p6_tp3);



end gates;

