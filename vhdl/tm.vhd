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
-- TM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tm is
    port (
      p2 : in  logicsig;
      p6 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end tm;
architecture gates of tm is
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

  component latchd2
    port (
      clk : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p15,
    y => r);


  u2 : g2 port map (
    a => p6,
    b => r,
    y2 => p9);


  u3 : g2 port map (
    a => p18,
    b => r,
    y2 => p19);


  u4 : g2 port map (
    a => p11,
    b => r,
    y2 => p8);


  u5 : g2 port map (
    a => p13,
    b => r,
    y2 => p10);


  u6 : g2 port map (
    a => p21,
    b => p16,
    y => a,
    y2 => e);

  p14 <= e;
  tp2 <= e;

  u7 : g2 port map (
    a => p16,
    b => p12,
    y => b,
    y2 => f);

  p17 <= f;
  tp5 <= f;

  u8 : g2 port map (
    a => p2,
    b => c,
    y => t1);

  p4 <= t1;

  u9 : rsflop port map (
    r => c,
    s => t1,
    q => tp1,
    qb => t2);


  u10 : g2 port map (
    a => t2,
    b => c,
    y => p5);


  u11 : g2 port map (
    a => a,
    b => b,
    y => t3,
    y2 => t4);

  p20 <= t4;
  p22 <= t4;
  tp3 <= t4;

  u12 : inv port map (
    a => p23,
    y => t5);

  tp4 <= t5;

  u13 : latchd2 port map (
    clk => p24,
    d => t3,
    d2 => t5,
    q => t6);

  tp6 <= t6;

  u14 : inv port map (
    a => t6,
    y => p28);


  u15 : g3 port map (
    a => b,
    b => t6,
    c => e,
    y => p27);


  u16 : g3 port map (
    a => a,
    b => t6,
    c => f,
    y => p26);


  u17 : g3 port map (
    a => t6,
    b => e,
    c => f,
    y => p25);



end gates;

