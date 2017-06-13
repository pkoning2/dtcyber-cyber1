-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- CJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cj is
    port (
      p6 : in  logicsig;
      p7 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3_tp2 : out logicsig;
      p5 : out logicsig;
      p8_tp1 : out logicsig;
      p9_tp3 : out logicsig;
      p10 : out logicsig;
      p14_tp4 : out logicsig;
      p17_tp5 : out logicsig;
      p19 : out logicsig;
      p21_tp6 : out logicsig;
      p28 : out logicsig);

end cj;
architecture gates of cj is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => p11,
    d => p12,
    q => t1);

  p9_tp3 <= t1;

  u2 : inv port map (
    a => t1,
    y => p10);


  u3 : latch port map (
    clk => p11,
    d => p7,
    q => t2);

  p8_tp1 <= t2;

  u4 : inv2 port map (
    a => t2,
    y => p5,
    y2 => t3);


  u5 : latch port map (
    clk => p11,
    d => p6,
    q => t4);

  p3_tp2 <= t4;

  u6 : inv2 port map (
    a => t4,
    y => p2,
    y2 => t5);

  p1 <= t5;

  u7 : g3 port map (
    a => p15,
    b => t3,
    c => t5,
    y => t6);


  u8 : g5 port map (
    a => t6,
    b => p26,
    c => p27,
    d => p25,
    e => p23,
    y2 => p28);


  u9 : latch port map (
    clk => p11,
    d => p22,
    q => p21_tp6);


  u10 : latch port map (
    clk => p11,
    d => p13,
    q => p14_tp4);


  u11 : latch port map (
    clk => p11,
    d => p20,
    q => p17_tp5);


  u12 : inv port map (
    a => p22,
    y => t10);


  u13 : inv port map (
    a => p20,
    y => t11);


  u14 : g3 port map (
    a => t10,
    b => p13,
    c => t11,
    y => p19);



end gates;

