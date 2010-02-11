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
-- IS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mod_is is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3_p5_p7 : out logicsig;
      p2 : out logicsig;
      p8_p12_p13_p14 : out logicsig;
      p10 : out logicsig;
      p15_p17_p19_p21 : out logicsig;
      p16 : out logicsig;
      p22_p24_p26_p28 : out logicsig;
      p27 : out logicsig);

end mod_is;
architecture gates of mod_is is
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

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p6,
    b => p4,
    y => a,
    y2 => t1);

  p2 <= t1;
  tp2 <= a;

  u2 : inv port map (
    a => t1,
    y => p1_p3_p5_p7);


  u3 : g3 port map (
    a => a,
    b => p18,
    c => p20,
    y => b,
    y2 => t3);

  p16 <= t3;
  tp5 <= b;

  u4 : inv port map (
    a => t3,
    y => p15_p17_p19_p21);


  u5 : g4 port map (
    a => b,
    b => p23,
    c => a,
    d => p25,
    y => t7,
    y2 => t5);

  p27 <= t5;
  tp6 <= t7;

  u6 : inv port map (
    a => t5,
    y => p22_p24_p26_p28);


  u7 : g5 port map (
    a => p9,
    b => t7,
    c => a,
    d => p11,
    e => b,
    y => tp1,
    y2 => t8);

  p10 <= t8;

  u8 : inv port map (
    a => t8,
    y => p8_p12_p13_p14);



end gates;

