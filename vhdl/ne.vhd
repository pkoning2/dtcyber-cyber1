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
-- NE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity neslice is
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

end neslice;
architecture gates of neslice is
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

entity ne is
    port (
      p1 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p8_tp1 : out logicsig;
      p10 : out logicsig;
      p10_tp2 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p20_tp5 : out logicsig;
      p22_tp6 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig);

end ne;
architecture gates of ne is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component neslice
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p17,
    y => c,
    y2 => d);


  u3 : neslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p28,
    i2 => p25,
    i3 => p27,
    y1 => p26,
    y2 => p24,
    y3 => t1);


  u5 : neslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => t1,
    i2 => p23,
    i3 => p21,
    y1 => p22_tp6,
    y2 => p20_tp5,
    y3 => t2);


  u6 : neslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => t2,
    i2 => p11,
    i3 => p19,
    y1 => p12,
    y2 => p18,
    y3 => t3);


  u7 : neslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => t3,
    i2 => p9,
    i3 => p5,
    y1 => p10_tp2,
    y2 => p8_tp1,
    y3 => t4);


  u8 : neslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => t4,
    i2 => p1,
    i3 => p7,
    y1 => p6,
    y2 => p2,
    y3 => p3);


  p15 <= '0';

end gates;

