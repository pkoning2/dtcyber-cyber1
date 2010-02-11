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
-- AJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity aj is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      p4_tp1 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p13_p15_p17_p19 : out logicsig;
      p14_p21_tp6 : out logicsig;
      p23 : out logicsig;
      p27 : out logicsig);

end aj;
architecture gates of aj is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component latch22
    port (
      clk : in  logicsig;
      clk2 : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u2 : g2 port map (
    a => p1,
    b => p3,
    y => t2);


  u3 : latch22 port map (
    clk => p7,
    clk2 => p22,
    d => a,
    d2 => t2,
    q => t4);

  p4_tp1 <= t4;
  tp2 <= a;

  u5 : g2 port map (
    a => t4,
    b => p10,
    y => p6);


  u7 : g2 port map (
    a => p28,
    b => p26,
    y => t7);


  u8 : latch22 port map (
    clk => p7,
    clk2 => p22,
    d => b,
    d2 => t7,
    q => t8,
    qb => p23);

  p14_p21_tp6 <= t8;
  tp5 <= b;

  u9 : inv port map (
    a => t8,
    y => p5);


  u10 : g2 port map (
    a => p24,
    b => t8,
    y => p27);


  u11 : rs2flop port map (
    r => t11,
    s => p11,
    s2 => p9,
    q => a);


  u13 : inv2 port map (
    a => p8,
    y => t11,
    y2 => p13_p15_p17_p19);


  u14 : rs2flop port map (
    r => t11,
    s => p20,
    s2 => p18,
    q => b);



end gates;

