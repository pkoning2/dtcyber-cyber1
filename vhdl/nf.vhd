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
-- NF module (variant of NE)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end nfslice;
architecture gates of nfslice is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => i2,
    b => i3,
    c => a,
    y => t1);


  u2 : g3 port map (
    a => i2,
    b => i3,
    c => b,
    y => y3);


  u3 : g3 port map (
    a => i1,
    b => t1,
    c => c,
    y => y1);


  u4 : g3 port map (
    a => i1,
    b => t1,
    c => d,
    y => y2);



end gates;

use work.sigs.all;

entity nf is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p2 : out logicsig;
      p6_tp2 : out logicsig;
      p8_tp3 : out logicsig;
      p10 : out logicsig;
      p13_p15 : out logicsig;
      p18 : out logicsig;
      p22_tp4 : out logicsig;
      p24_tp6 : out logicsig;
      p26_tp5 : out logicsig;
      p28 : out logicsig);

end nf;
architecture gates of nf is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component nfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal p4_tp1 : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : nfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p7,
    i2 => p9,
    i3 => p11,
    y1 => p8_tp3,
    y2 => p6_tp2,
    y3 => t1);


  u2 : nfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => t1,
    i2 => p5,
    i3 => p3,
    y1 => p2,
    y2 => p4_tp1,
    y3 => p10);


  u3 : nfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p17,
    i2 => p19,
    i3 => p21,
    y1 => p22_tp4,
    y2 => p24_tp6,
    y3 => t2);


  u4 : nfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => t2,
    i2 => p23,
    i3 => p25,
    y1 => p26_tp5,
    y2 => p28,
    y3 => p18);


  u5 : inv2 port map (
    a => p20,
    y => a,
    y2 => b);


  u6 : inv2 port map (
    a => p1,
    y => c,
    y2 => d);


  p13_p15 <= '0';

end gates;

