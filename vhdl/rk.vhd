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
-- RK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rk is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p9 : out logicsig;
      p10_tp3 : out logicsig;
      p11_tp2 : out logicsig;
      p12_tp1 : out logicsig;
      p15_tp4 : out logicsig;
      p17 : out logicsig;
      p19_tp5 : out logicsig;
      p20_tp6 : out logicsig);

end rk;
architecture gates of rk is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : inv port map (
    a => p13,
    y => x);


  u2 : rs4flop port map (
    r => x,
    s => p1,
    s2 => p3,
    s3 => p5,
    s4 => p7,
    q => p12_tp1);


  u3 : latch port map (
    clk => p18,
    d => p16,
    q => t1);

  p15_tp4 <= t1;

  u4 : inv2 port map (
    a => t1,
    y2 => p17);


  u5 : latch port map (
    clk => p18,
    d => p14,
    q => t2);

  p11_tp2 <= t2;

  u6 : inv2 port map (
    a => t2,
    y2 => p9);


  u7 : rs4flop port map (
    r => x,
    s => p21,
    s2 => p23,
    s3 => p25,
    s4 => p27,
    q => p19_tp5);


  u8 : rs4flop port map (
    r => x,
    s => p8,
    s2 => p6,
    s3 => p4,
    s4 => p2,
    q => p10_tp3);


  u9 : rs4flop port map (
    r => x,
    s => p28,
    s2 => p26,
    s3 => p24,
    s4 => p22,
    q => p20_tp6);



end gates;

