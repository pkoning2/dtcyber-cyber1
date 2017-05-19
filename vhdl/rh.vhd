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
-- RH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rh is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp4 : out logicsig;
      p1_tp2 : out logicsig;
      p5_tp1 : out logicsig;
      p14_tp3 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p24_tp6 : out logicsig;
      p26 : out logicsig;
      p28_tp5 : out logicsig);

end rh;
architecture gates of rh is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t20 : logicsig;
  signal t21 : logicsig;
  signal t22 : logicsig;
  signal t23 : logicsig;
  signal t24 : logicsig;
  signal t25 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p4,
    y2 => t1);


  u2 : g2 port map (
    a => p2,
    b => t1,
    y => c);


  u3 : g2 port map (
    a => p8,
    b => t1,
    y => d);


  u4 : inv port map (
    a => p2,
    y => t2);


  u5 : inv2 port map (
    a => p7,
    y2 => t3);


  u6 : inv port map (
    a => p8,
    y => t4);


  u7 : g2 port map (
    a => t2,
    b => t3,
    y => t5);


  u8 : g2 port map (
    a => t3,
    b => t4,
    y => t6);


  u9 : g2 port map (
    a => c,
    b => t5,
    y => p1_tp2);


  u10 : g2 port map (
    a => d,
    b => t6,
    y => p5_tp1);


  u11 : g2 port map (
    a => p17,
    b => p9,
    y => t10);


  u12 : g2 port map (
    a => p18,
    b => p13,
    y => t11);


  u13 : g2 port map (
    a => p16,
    b => p10,
    y => t12);


  u14 : g2 port map (
    a => p11,
    b => p15,
    y => t13);


  u15 : g4 port map (
    a => t10,
    b => t11,
    c => t12,
    d => t13,
    y => p14_tp3);


  u16 : inv2 port map (
    a => p25,
    y2 => t20);

  p26 <= p25;

  u17 : g2 port map (
    a => p27,
    b => t20,
    y => a);


  u18 : g2 port map (
    a => p23,
    b => t20,
    y => b);


  u19 : inv port map (
    a => p27,
    y => t21);

  p21 <= t21;

  u20 : inv2 port map (
    a => p20,
    y2 => t22);

  p19 <= p20;
  tp4 <= t22;

  u21 : inv port map (
    a => p23,
    y => t23);

  p22 <= t23;

  u22 : g2 port map (
    a => t21,
    b => t22,
    y => t24);


  u23 : g2 port map (
    a => t24,
    b => a,
    y => p28_tp5);


  u24 : g2 port map (
    a => t22,
    b => t23,
    y => t25);


  u25 : g2 port map (
    a => t25,
    b => b,
    y => p24_tp6);



end gates;

