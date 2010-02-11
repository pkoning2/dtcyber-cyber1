-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- AB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity abslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      q1_q2_q3 : out logicsig);

end abslice;
architecture gates of abslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);


  u2 : inv port map (
    a => a,
    y => t2);


  u3 : inv port map (
    a => b,
    y => t3);


  u4 : g2 port map (
    a => t2,
    b => t3,
    y => t4);


  u5 : g2 port map (
    a => t1,
    b => t4,
    y => q1_q2_q3);



end gates;

use work.sigs.all;

entity ab is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      p4_p6_tp2 : out logicsig;
      p10_p12_tp1 : out logicsig;
      p14_p16_tp3 : out logicsig;
      p15_p17_tp4 : out logicsig;
      p19_p21_tp6 : out logicsig;
      p25_p27_tp5 : out logicsig);

end ab;
architecture gates of ab is
  component abslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      q1_q2_q3 : out logicsig);

  end component;


begin -- gates
  u1 : abslice port map (
    a => p5,
    b => p8,
    q1_q2_q3 => p10_p12_tp1);


  u2 : abslice port map (
    a => p18,
    b => p20,
    q1_q2_q3 => p15_p17_tp4);


  u3 : abslice port map (
    a => p1,
    b => p3,
    q1_q2_q3 => p4_p6_tp2);


  u4 : abslice port map (
    a => p28,
    b => p26,
    q1_q2_q3 => p25_p27_tp5);


  u5 : abslice port map (
    a => p13,
    b => p11,
    q1_q2_q3 => p14_p16_tp3);


  u6 : abslice port map (
    a => p24,
    b => p23,
    q1_q2_q3 => p19_p21_tp6);



end gates;

