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
-- DY module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity dy is
    port (
      p5 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  coaxsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p22 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end dy;
architecture gates of dy is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

  end component;

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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t19 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => p16,
    d => p8,
    q => t1);

  tp1 <= t1;

  u2 : inv2 port map (
    a => t1,
    y => e,
    y2 => t2);

  p1 <= t2;
  p2 <= t2;
  p3 <= t2;
  p4 <= e;
  p6 <= e;

  u3 : latch port map (
    clk => p16,
    d => p5,
    q => t3);

  tp2 <= t3;

  u4 : inv port map (
    a => t3,
    y => t4);


  u5 : g2 port map (
    a => t4,
    b => d,
    y => t5);

  p14 <= t5;

  u6 : g2 port map (
    a => t5,
    b => f,
    y => p7);

  p21 <= f;
  tp6 <= f;

  u7 : latch port map (
    clk => p16,
    d => p17,
    q => t6);

  tp5 <= t6;

  u8 : inv2 port map (
    a => t6,
    y => t7,
    y2 => t8);

  p24 <= t8;
  p25 <= t7;
  p26 <= t8;
  p27 <= t7;

  u9 : inv2 port map (
    a => p18,
    y => f,
    y2 => t19);


  u10 : g3 port map (
    a => t6,
    b => e,
    c => f,
    y => p15,
    y2 => p28);


  u11 : g2 port map (
    a => t9,
    b => p13,
    y => p23);

  p9 <= t9;
  p19 <= t9;
  p20 <= t9;

  u12 : g3 port map (
    a => t9,
    b => e,
    c => p13,
    y => t10);

  p11 <= t10;
  tp3 <= t10;

  u13 : cxreceiver port map (
    a => p10,
    y => t11);


  u14 : rsflop port map (
    r => p22,
    s => t11,
    q => t12);

  p12 <= t12;

  u15 : inv port map (
    a => t12,
    y => d);



end gates;

