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
-- IY module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity iy is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p9 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end iy;
architecture gates of iy is
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
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t18 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p2,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => p15,
    y2 => t2);

  p13 <= t2;
  tp2 <= t2;

  u3 : inv port map (
    a => p15,
    y => t3);


  u4 : g2 port map (
    a => t1,
    b => t3,
    y => t4);


  u5 : inv port map (
    a => p18,
    y => t5);

  p20 <= t5;
  tp5 <= t5;

  u6 : inv port map (
    a => p19,
    y => t6);

  p17 <= t6;

  u7 : g2 port map (
    a => t5,
    b => t6,
    y => t7);


  u8 : g2 port map (
    a => t4,
    b => t7,
    y => t8);

  tp4 <= t8;
  p16 <= t8;

  u9 : inv port map (
    a => p23,
    y => t9);


  u10 : g2 port map (
    a => t5,
    b => t9,
    y => t10);


  u11 : g2 port map (
    a => p25,
    b => p27,
    y => t11);


  u12 : g4 port map (
    a => t10,
    b => p22,
    c => p24,
    d => t11,
    y => t12);

  p26 <= t12;
  tp6 <= t12;

  u13 : inv2 port map (
    a => p27,
    y => t13,
    y2 => p28);


  u14 : g5 port map (
    a => p10,
    b => p6,
    c => p18,
    d => p4,
    e => t13,
    y => t14);


  u15 : rsflop port map (
    r => p8,
    s => t14,
    q => t15);

  tp1 <= t15;

  u16 : inv port map (
    a => t15,
    y => t16);

  p1 <= t16;
  p3 <= t16;
  p9 <= t16;

  u17 : inv port map (
    a => p11,
    y => p21);


  u18 : inv port map (
    a => p12,
    y => t18);

  p14 <= t18;
  tp3 <= t18;


end gates;

