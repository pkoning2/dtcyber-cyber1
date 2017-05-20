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
-- KG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kg is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      p10_p8_p6_p4_p2 : out logicsig;
      p11_tp1 : out logicsig;
      p16_tp4 : out logicsig;
      p26_tp5 : out logicsig;
      p27_tp6 : out logicsig);

end kg;
architecture gates of kg is
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

  component rs4flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal x : logicsig;

begin -- gates
  u1 : rs4flop port map (
    r => x,
    s => p1,
    s2 => p3,
    s3 => p9,
    s4 => p13,
    q => p11_tp1);

  tp3 <= x;

  u2 : rs4flop port map (
    r => x,
    s => p12,
    s2 => p14,
    s3 => p15,
    s4 => p17,
    q => p16_tp4);


  u3 : rs4flop port map (
    r => x,
    s => p20,
    s2 => p22,
    s3 => p24,
    s4 => p28,
    q => p26_tp5);


  u4 : rs4flop port map (
    r => x,
    s => p19,
    s2 => p21,
    s3 => p23,
    s4 => p25,
    q => p27_tp6);


  u5 : inv port map (
    a => p18,
    y => x);


  u6 : g2 port map (
    a => p5,
    b => p7,
    y => tp2,
    y2 => p10_p8_p6_p4_p2);



end gates;

