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
-- HY module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hy is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3_p5_p7 : out logicsig;
      p2 : out logicsig;
      p8 : out logicsig;
      p12 : out logicsig;
      p14_tp1 : out logicsig;
      p15_p17_p19_p21 : out logicsig;
      p16 : out logicsig;
      p22_p24_p26_p28 : out logicsig;
      p27 : out logicsig);

end hy;
architecture gates of hy is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p13,
    b => p10,
    y => p14_tp1);


  u2 : g2 port map (
    a => p4,
    b => p6,
    y => a,
    y2 => t2);

  p2 <= t2;
  tp2 <= a;

  u3 : inv port map (
    a => t2,
    y => p1_p3_p5_p7);


  u4 : g3 port map (
    a => a,
    b => p18,
    c => p20,
    y => b,
    y2 => t4);

  p16 <= t4;
  tp5 <= b;

  u5 : inv port map (
    a => t4,
    y => p15_p17_p19_p21);


  u6 : g2 port map (
    a => t4,
    b => p11,
    y => p12);


  u7 : g4 port map (
    a => b,
    b => a,
    c => p23,
    d => p25,
    y => tp6,
    y2 => t7);

  p27 <= t7;

  u8 : inv port map (
    a => t7,
    y => t8);

  p22_p24_p26_p28 <= t8;

  u9 : g2 port map (
    a => t8,
    b => p9,
    y => p8);



end gates;

