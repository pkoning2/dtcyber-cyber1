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
-- HW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hw is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p28 : out logicsig);

end hw;
architecture gates of hw is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p1,
    y2 => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => t1,
    b => p3,
    y => p2);


  u3 : g2 port map (
    a => t1,
    b => p5,
    y => p4);


  u4 : g2 port map (
    a => p7,
    b => t1,
    y => p6);


  u5 : g2 port map (
    a => p9,
    b => p11,
    y => t2);

  tp2 <= t2;

  u6 : g2 port map (
    a => t2,
    b => p8,
    y => p10);


  u7 : g2 port map (
    a => t2,
    b => p12,
    y => p13);


  u8 : inv2 port map (
    a => p25,
    y2 => t3);

  tp5 <= t3;

  u9 : g2 port map (
    a => p23,
    b => t3,
    y => p28);


  u10 : g2 port map (
    a => t3,
    b => p26,
    y => p21);


  u11 : g2 port map (
    a => p24,
    b => t3,
    y => p19);


  u12 : g2 port map (
    a => p18,
    b => p17,
    y => t4);

  tp6 <= t4;

  u13 : g2 port map (
    a => p20,
    b => t4,
    y => p22);


  u14 : g2 port map (
    a => p14,
    b => t4,
    y => p16);



end gates;

