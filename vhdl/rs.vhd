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
-- RS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rs is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p3_tp1 : out logicsig;
      p6_tp2 : out logicsig;
      p14_tp3 : out logicsig;
      p16_tp6 : out logicsig;
      p20_p15 : out logicsig;
      p22_p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end rs;
architecture gates of rs is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t20 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p9,
    y => t1);


  u2 : inv2 port map (
    a => p5,
    y2 => t2);


  u3 : g2 port map (
    a => p7,
    b => t2,
    y => t3);


  u4 : g3 port map (
    a => t1,
    b => p12,
    c => t3,
    y => p14_tp3);


  u5 : g2 port map (
    a => t2,
    b => p18,
    y => p16_tp6);


  u6 : inv port map (
    a => p8,
    y => t10);


  u7 : inv2 port map (
    a => p2,
    y2 => t11);


  u8 : g2 port map (
    a => p4,
    b => t11,
    y => t12);


  u9 : g3 port map (
    a => t10,
    b => p1,
    c => t12,
    y => p3_tp1);


  u10 : g2 port map (
    a => t11,
    b => p10,
    y => p6_tp2);


  u11 : g2 port map (
    a => p13,
    b => p11,
    y => tp5,
    y2 => p20_p15);


  u12 : g4 port map (
    a => p19,
    b => p21,
    c => p23,
    d => p25,
    y => t20,
    y2 => p26);

  tp4 <= t20;

  u13 : g2 port map (
    a => t20,
    b => p17,
    y => p22_p24);


  u14 : g2 port map (
    a => t20,
    b => p27,
    y => p28);



end gates;

