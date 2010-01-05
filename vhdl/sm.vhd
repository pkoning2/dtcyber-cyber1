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
-- SM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity smslice is
    port (
      clk : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

end smslice;
architecture gates of smslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => clk,
    d => d,
    q => t1);

  tp <= t1;

  u2 : inv2 port map (
    a => t1,
    y => qb,
    y2 => t2);

  q1 <= t2;
  q2 <= t2;


end gates;

use work.sigs.all;

entity sm is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end sm;
architecture gates of sm is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component smslice
    port (
      clk : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : smslice port map (
    clk => p7,
    d => p8,
    q1 => p1,
    q2 => p3,
    qb => p4,
    tp => tp1);


  u2 : smslice port map (
    clk => p7,
    d => p5,
    q1 => p2,
    q2 => p6,
    qb => p11,
    tp => tp2);


  u3 : smslice port map (
    clk => p7,
    d => p12,
    q1 => p14,
    q2 => p10,
    qb => p13,
    tp => tp3);


  u4 : smslice port map (
    clk => p20,
    d => p21,
    q1 => p19,
    q2 => p17,
    qb => p15,
    tp => tp4);


  u5 : smslice port map (
    clk => p20,
    d => p24,
    q1 => p27,
    q2 => p18,
    qb => p16,
    tp => tp5);


  u6 : latch port map (
    clk => p20,
    d => p9,
    q => t1);


  u7 : inv port map (
    a => t1,
    y => x);


  u8 : latch port map (
    clk => p20,
    d => p23,
    q => t2);

  p25 <= t2;
  tp6 <= t2;

  u9 : inv2 port map (
    a => t2,
    y => t3,
    y2 => p28);


  u10 : g2 port map (
    a => t3,
    b => x,
    y => t4);

  p22 <= t4;
  p26 <= t4;


end gates;

