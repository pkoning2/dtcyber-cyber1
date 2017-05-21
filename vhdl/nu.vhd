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
-- NU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nuslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

end nuslice;
architecture gates of nuslice is
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
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => i2,
    y => t1);


  u2 : g2 port map (
    a => i3,
    b => i4,
    y => t2);


  u3 : g2 port map (
    a => a,
    b => t1,
    y => t4);


  u4 : g2 port map (
    a => c,
    b => t2,
    y => t5);


  u5 : g3 port map (
    a => t4,
    b => b,
    c => t5,
    y => y);


  u6 : g2 port map (
    a => t4,
    b => t5,
    y => t3);


  u7 : g2 port map (
    a => t3,
    b => d,
    y => y2);



end gates;

use work.sigs.all;

entity nu is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p1_tp1 : out logicsig;
      p7_tp2 : out logicsig;
      p8 : out logicsig;
      p11_tp3 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22_tp4 : out logicsig;
      p23_tp5 : out logicsig);

end nu;
architecture gates of nu is
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

  component nuslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv port map (
    a => p14,
    y => b);


  u2 : inv2 port map (
    a => p13,
    y => a,
    y2 => c);


  u3 : inv port map (
    a => p15,
    y => d);


  u4 : nuslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p26,
    i2 => p28,
    i3 => p27,
    i4 => p25,
    y => p23_tp5,
    y2 => p21);


  u5 : nuslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p17,
    i2 => p19,
    i3 => p16,
    i4 => p18,
    y => p22_tp4,
    y2 => p20);


  u6 : nuslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p5,
    i2 => p3,
    i3 => p4,
    i4 => p2,
    y => p1_tp1,
    y2 => p7_tp2);


  u7 : nuslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p6,
    i2 => p9,
    i3 => p12,
    i4 => p10,
    y => p11_tp3,
    y2 => p8);



end gates;

