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
-- ML module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ml is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p27 : in  logicsig;
      p1_tp2 : out coaxsig;
      p4_tp1 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p14_tp3 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18_tp4 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22_tp5 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26_tp6 : out logicsig;
      p28 : out logicsig);

end ml;
architecture gates of ml is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => p9,
    d => f,
    q => t1,
    qb => p17);

  p20 <= t1;
  p22_tp5 <= f;

  u2 : latch port map (
    clk => p15,
    d => t1,
    q => t2,
    qb => p13);

  p18_tp4 <= t2;

  u3 : latch port map (
    clk => p9,
    d => t2,
    q => t3,
    qb => p11);

  p16 <= t3;

  u5 : latch port map (
    clk => p9,
    d => p27,
    q => t4,
    qb => p25);

  p28 <= t4;

  u6 : latch port map (
    clk => p15,
    d => t4,
    q => t5,
    qb => p23);

  p26_tp6 <= t5;

  u7 : latch port map (
    clk => p9,
    d => t5,
    q => t6,
    qb => p21);

  p24 <= t6;

  u8 : latch port map (
    clk => p15,
    d => t6,
    q => f,
    qb => p19);


  u9 : g5 port map (
    a => p6,
    b => p12,
    c => p10,
    d => p8,
    e => p2,
    y => p4_tp1);


  u10 : g2 port map (
    a => p7,
    b => p5,
    y => e);


  u11 : inv2 port map (
    a => p15,
    y2 => d);


  u12 : g2 port map (
    a => t3,
    b => d,
    y => t9);


  u13 : rsflop port map (
    r => e,
    s => t9,
    q => p14_tp3);


  u14 : cxdriver port map (
    a => p3,
    y => p1_tp2);



end gates;

