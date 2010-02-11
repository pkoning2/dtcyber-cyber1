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
-- AO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ao is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2_tp5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15_tp3 : out logicsig;
      p19 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end ao;
architecture gates of ao is
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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
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
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal j : logicsig;
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

begin -- gates
  u1 : g2 port map (
    a => p10,
    b => p11,
    y => a);


  u2 : rsflop port map (
    r => p17,
    s => a,
    q => t1,
    qb => j);

  p2_tp5 <= t1;

  u3 : g2 port map (
    a => t1,
    b => c,
    y => t2);

  p14 <= t2;

  u4 : rs2flop port map (
    r => p5,
    s => t2,
    s2 => g,
    q => t3);

  p24 <= g;
  tp4 <= t3;

  u5 : g2 port map (
    a => p10,
    b => t3,
    y => t4);


  u6 : rs2flop port map (
    r => p17,
    s => t4,
    s2 => a,
    q => t5);

  p15_tp3 <= t5;

  u7 : inv port map (
    a => t5,
    y => t6);


  u8 : inv port map (
    a => p8,
    y => t7);


  u9 : rsflop port map (
    r => t7,
    s => a,
    q => t8);

  tp1 <= t8;

  u10 : g2 port map (
    a => t6,
    b => t8,
    y => p7);


  u11 : inv port map (
    a => p4,
    y => t9);


  u12 : rsflop port map (
    r => p5,
    s => t9,
    q => d);

  tp2 <= d;

  u13 : g2 port map (
    a => d,
    b => c,
    y2 => p6);


  u14 : g4 port map (
    a => d,
    b => p25,
    c => p26,
    d => p27,
    y => p28);


  u15 : g3 port map (
    a => d,
    b => p3,
    c => e,
    y => p1);

  tp6 <= e;

  u16 : inv2 port map (
    a => p9,
    y => b,
    y2 => c);


  u17 : g2 port map (
    a => p16,
    b => p18,
    y => g);


  u18 : g2 port map (
    a => p18,
    b => p12,
    y => t10);


  u19 : g6 port map (
    a => j,
    b => p20,
    c => t10,
    d => p21,
    e => p22,
    f => p23,
    y => t11);


  u20 : rsflop port map (
    r => t11,
    s => a,
    q => e,
    qb => p13);


  u21 : g3 port map (
    a => d,
    b => e,
    c => c,
    y => p19);



end gates;

