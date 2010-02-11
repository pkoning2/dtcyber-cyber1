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
-- GF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gfslice is
    port (
      f : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      k : in  logicsig;
      q1_q2_q3_tp : out logicsig);

end gfslice;
architecture gates of gfslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal ta : logicsig;

begin -- gates
  u1 : g2 port map (
    a => in1,
    b => k,
    y => t1);


  u2 : g2 port map (
    a => in2,
    b => t1,
    y => ta);


  u3 : g2 port map (
    a => ta,
    b => f,
    y => q1_q2_q3_tp);



end gates;

use work.sigs.all;

entity gf is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp4 : out logicsig;
      p3_p5_p7_tp2 : out logicsig;
      p4_p6_p8_tp1 : out logicsig;
      p13_p15_p17_tp3 : out logicsig;
      p21_p23_p25_tp6 : out logicsig;
      p22_p24_p26_tp5 : out logicsig);

end gf;
architecture gates of gf is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component gfslice
    port (
      f : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      k : in  logicsig;
      q1_q2_q3_tp : out logicsig);

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

  signal f : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : gfslice port map (
    f => f,
    in1 => p1,
    in2 => p2,
    k => k,
    q1_q2_q3_tp => p3_p5_p7_tp2);


  u2 : gfslice port map (
    f => f,
    in1 => p10,
    in2 => p9,
    k => k,
    q1_q2_q3_tp => p4_p6_p8_tp1);


  u3 : gfslice port map (
    f => f,
    in1 => p28,
    in2 => p27,
    k => k,
    q1_q2_q3_tp => p22_p24_p26_tp5);


  u4 : gfslice port map (
    f => f,
    in1 => p19,
    in2 => p20,
    k => k,
    q1_q2_q3_tp => p21_p23_p25_tp6);


  u5 : gfslice port map (
    f => f,
    in1 => p11,
    in2 => p12,
    k => k,
    q1_q2_q3_tp => p13_p15_p17_tp3);


  u6 : inv2 port map (
    a => p14,
    y2 => k);


  u7 : inv port map (
    a => k,
    y => t1);


  u8 : g2 port map (
    a => p16,
    b => t1,
    y => j);

  tp4 <= j;

  u9 : g2 port map (
    a => p18,
    b => p16,
    y => i);


  u10 : g2 port map (
    a => i,
    b => j,
    y => f);



end gates;

