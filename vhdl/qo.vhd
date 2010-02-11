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
-- QO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qo is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p11_tp3 : out logicsig;
      p12 : out logicsig;
      p13_tp4 : out logicsig;
      p20 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig);

end qo;
architecture gates of qo is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p10,
    y => t1,
    y2 => t2);

  tp1 <= t2;

  u2 : g2 port map (
    a => t1,
    b => p9,
    y => t3);


  u3 : g2 port map (
    a => t2,
    b => p8,
    y => t4);


  u4 : g2 port map (
    a => t3,
    b => t4,
    y => p12);


  u5 : inv2 port map (
    a => p7,
    y => t5,
    y2 => t6);

  tp2 <= t6;

  u6 : g2 port map (
    a => p6,
    b => t5,
    y => t7);


  u7 : g2 port map (
    a => t6,
    b => p5,
    y => t8);


  u8 : g2 port map (
    a => t7,
    b => t8,
    y => p4);


  u9 : g3 port map (
    a => p3,
    b => p1,
    c => p2,
    y => p11_tp3);


  u10 : g3 port map (
    a => p14,
    b => p16,
    c => p18,
    y => p13_tp4);


  u11 : rsflop port map (
    r => p27,
    s => p28,
    q => t11);

  tp5 <= t11;

  u12 : inv2 port map (
    a => t11,
    y => p25,
    y2 => t12);


  u13 : inv port map (
    a => t12,
    y => p23);


  u14 : inv port map (
    a => p24,
    y => t13);


  u15 : inv port map (
    a => p22,
    y => t14);


  u16 : g6 port map (
    a => t13,
    b => p15,
    c => p21,
    d => p19,
    e => t14,
    f => p17,
    y => tp6,
    y2 => p20);



end gates;

