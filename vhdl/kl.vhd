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
-- KL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kl is
    port (
      p2 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end kl;
architecture gates of kl is
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
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p2,
    b => p14,
    y => t1);

  p17 <= t1;
  tp2 <= t1;

  u2 : inv2 port map (
    a => t1,
    y2 => t2);

  p3 <= t2;
  p4 <= t2;
  p5 <= t2;
  p6 <= t2;

  u3 : g2 port map (
    a => p14,
    b => p16,
    y => t3);

  p20 <= t3;
  tp1 <= t3;

  u4 : inv2 port map (
    a => t3,
    y2 => t4);

  p7 <= t4;
  p8 <= t4;
  p9 <= t4;
  p10 <= t4;

  u5 : g2 port map (
    a => p27,
    b => p15,
    y => t5);

  p11 <= t5;
  tp5 <= t5;

  u6 : inv2 port map (
    a => t5,
    y2 => t6);

  p23 <= t6;
  p25 <= t6;
  p26 <= t6;
  p28 <= t6;

  u7 : g2 port map (
    a => p15,
    b => p13,
    y => t7);

  p12 <= t7;
  tp6 <= t7;

  u8 : inv2 port map (
    a => t7,
    y2 => t8);

  p19 <= t8;
  p21 <= t8;
  p22 <= t8;
  p24 <= t8;


end gates;

