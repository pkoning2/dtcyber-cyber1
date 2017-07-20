-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- CU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cu is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      p4_tp1 : out logicsig;
      p5_tp4 : out logicsig;
      p6 : out logicsig;
      p9_tp3 : out logicsig;
      p10_tp5 : out logicsig;
      p14 : out logicsig;
      p16_tp6 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p26_p28 : out logicsig);

end cu;
architecture gates of cu is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig;
      qs : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal d : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : rsflop port map (
    r => p3,
    s => g,
    q => t1);


  u2 : g5 port map (
    a => t1,
    b => a,
    c => d,
    d => p1,
    e => f,
    y => t2);

  p9_tp3 <= a;
  tp2 <= t2;

  u3 : g2 port map (
    a => t2,
    b => p2,
    y => t3);


  u4 : g2 port map (
    a => t3,
    b => p8,
    y => t4);


  u5 : rsflop port map (
    r => p3,
    s => t4,
    q => p4_tp1,
    qb => p6);


  u6 : latch port map (
    clk => p11,
    d => p12,
    q => a,
    qb => b);


  u7 : latch port map (
    clk => p11,
    d => p13,
    q => p5_tp4,
    qb => d);


  u8 : latch port map (
    clk => p11,
    d => p7,
    q => p10_tp5,
    qb => f);


  u9 : g4 port map (
    a => p23,
    b => b,
    c => f,
    d => d,
    y => t6);

  p20 <= t6;

  u10 : g2 port map (
    a => p17,
    b => p15,
    y => t7,
    y2 => t8);


  u11 : g2 port map (
    a => t8,
    b => p19,
    y => p21);


  u12 : g4 port map (
    a => t6,
    b => p24,
    c => p22,
    d => t7,
    y => p16_tp6);


  u13 : inv port map (
    a => p18,
    y => p14);


  u14 : g2 port map (
    a => p27,
    b => p25,
    y2 => p26_p28);


  u15 : inv port map (
    a => p11,
    y => g);
  

end gates;

