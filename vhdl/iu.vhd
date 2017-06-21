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
-- IU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity iu is
    port (
      p2 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p10_tp1 : out logicsig;
      p11 : out logicsig;
      p13_tp2 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18_tp5 : out logicsig;
      p19_tp6 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end iu;
architecture gates of iu is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p14,
    y => a);

  p16 <= a;

  u2 : inv port map (
    a => p12,
    y => b);

  p10_tp1 <= b;

  u3 : inv port map (
    a => p8,
    y => c);

  p7 <= c;

  u4 : inv port map (
    a => p5,
    y => d);

  p6 <= d;

  u5 : inv port map (
    a => p2,
    y => e);

  p4 <= e;

  u6 : g2 port map (
    a => p21,
    b => p22,
    y => p19_tp6,
    y2 => t2);


  u7 : g2 port map (
    a => t2,
    b => a,
    y => p13_tp2);


  u8 : g2 port map (
    a => t2,
    b => b,
    y => p11);


  u9 : g2 port map (
    a => t2,
    b => c,
    y => p9);


  u10 : g2 port map (
    a => t2,
    b => d,
    y => p3);


  u11 : g2 port map (
    a => t2,
    b => e,
    y => p1);


  u12 : g2 port map (
    a => t2,
    b => p20,
    y => p18_tp5);


  u13 : g2 port map (
    a => t2,
    b => p23,
    y => p17);


  u14 : g2 port map (
    a => t2,
    b => p25,
    y => p15);


  u15 : g2 port map (
    a => t2,
    b => p26,
    y => p24);


  u16 : g2 port map (
    a => t2,
    b => p27,
    y => p28);



end gates;

