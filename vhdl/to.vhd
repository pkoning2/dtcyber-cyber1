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
-- TO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mod_to is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end mod_to;
architecture gates of mod_to is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p8,
    y => t1);


  u2 : g2 port map (
    a => p7,
    b => t1,
    y => t2);


  u3 : g2 port map (
    a => p10,
    b => t2,
    y => p12);


  u4 : g2 port map (
    a => p5,
    b => t1,
    y => t3);


  u5 : g3 port map (
    a => t3,
    b => p6,
    c => p4,
    y => p15);


  u6 : g2 port map (
    a => p14,
    b => p16,
    y => tp5,
    y2 => t4);

  tp6 <= t4;

  u7 : inv port map (
    a => t4,
    y => t5);

  p17 <= t5;
  p18 <= t5;
  p19 <= t5;
  p20 <= t5;
  p21 <= t5;
  p22 <= t5;
  p23 <= t5;
  p24 <= t5;
  p25 <= t5;
  p26 <= t5;
  p27 <= t5;
  p28 <= t5;

  u8 : inv2 port map (
    a => p2,
    y2 => t6);

  p9 <= t6;
  p11 <= t6;
  p13 <= t6;

  u9 : inv port map (
    a => p3,
    y => p1);



end gates;

