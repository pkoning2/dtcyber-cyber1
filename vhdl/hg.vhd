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
-- HG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hg is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p8_tp2 : out logicsig;
      p10_p12 : out logicsig;
      p18 : out logicsig;
      p20_p21 : out logicsig;
      p22 : out logicsig;
      p24_tp6 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end hg;
architecture gates of hg is
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
      qb : out logicsig;
      qs : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal m : logicsig;
  signal t1 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => p14,
    d => p3,
    q => g,
    qb => c);

  tp1 <= g;

  u2 : g2 port map (
    a => p5,
    b => p9,
    y => t1);


  u3 : g2 port map (
    a => p3,
    b => t1,
    y => p8_tp2);


  u4 : g2 port map (
    a => p9,
    b => p7,
    y => t3);


  u5 : g2 port map (
    a => t3,
    b => p19,
    y => m,
    y2 => p18);


  u6 : latch port map (
    clk => p14,
    d => p15,
    q => t4);

  tp3 <= t4;

  u7 : inv port map (
    a => t4,
    y => t5);


  u8 : g2 port map (
    a => g,
    b => t5,
    y2 => p4);


  u9 : g2 port map (
    a => g,
    b => p2,
    y2 => p1);


  u10 : inv2 port map (
    a => p14,
    y => a,
    y2 => b);


  u11 : g2 port map (
    a => p23,
    b => b,
    y => f);


  u12 : latch port map (
    clk => p14,
    d => p23,
    q => p24_tp6,
    qb => e);


  u14 : g3 port map (
    a => p17,
    b => m,
    c => p16,
    y => t7);

  tp5 <= t7;

  u15 : latch port map (
    clk => p14,
    d => t7,
    q => t8);

  tp4 <= t8;

  u16 : inv port map (
    a => t8,
    y => p10_p12);


  u17 : inv port map (
    a => p25,
    y => t10);

  p26 <= t10;

  u18 : g2 port map (
    a => t10,
    b => p27,
    y2 => p28);


  u19 : g2 port map (
    a => e,
    b => f,
    y => p22,
    y2 => p20_p21);



end gates;

