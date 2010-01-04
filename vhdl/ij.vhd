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
-- IJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ij is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ij;
architecture gates of ij is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g5 port map (
    a => p12,
    b => p14,
    c => p13,
    d => p16,
    e => p15,
    y => t1);

  p11 <= t1;
  tp2 <= t1;

  u2 : g2 port map (
    a => p17,
    b => p18,
    y => tp5,
    y2 => t2);


  u3 : g2 port map (
    a => t2,
    b => p10,
    y => t3);

  p9 <= t3;
  tp1 <= t3;

  u4 : g2 port map (
    a => t2,
    b => p8,
    y => p7);


  u5 : g2 port map (
    a => t2,
    b => p6,
    y => p5);


  u6 : g2 port map (
    a => t2,
    b => p4,
    y => p3);


  u7 : g2 port map (
    a => t2,
    b => p2,
    y => p1);


  u8 : g2 port map (
    a => t2,
    b => p19,
    y => t4);

  p20 <= t4;
  tp6 <= t4;

  u9 : g2 port map (
    a => t2,
    b => p21,
    y => p22);


  u10 : g2 port map (
    a => t2,
    b => p23,
    y => p24);


  u12 : g2 port map (
    a => p25,
    b => t2,
    y => p26);


  u13 : g2 port map (
    a => t2,
    b => p27,
    y => p28);



end gates;

