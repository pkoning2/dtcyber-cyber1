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
-- QQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qq is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p8_p9 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p22 : out logicsig;
      p28 : out logicsig);

end qq;
architecture gates of qq is
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

  component rs4flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
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
  signal t19 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p2,
    y => t1,
    y2 => t2);


  u2 : g2 port map (
    a => t1,
    b => p1,
    y => t3);


  u3 : g2 port map (
    a => t2,
    b => p3,
    y => t4);


  u4 : inv port map (
    a => p27,
    y => t5);


  u5 : inv2 port map (
    a => p4,
    y => t6,
    y2 => t7);


  u6 : g2 port map (
    a => p5,
    b => t6,
    y => t8);


  u7 : g2 port map (
    a => t7,
    b => p7,
    y => t9);


  u8 : g5 port map (
    a => t3,
    b => t4,
    c => t5,
    d => t8,
    e => t9,
    y => t10);


  u9 : g2 port map (
    a => p23,
    b => t10,
    y => t11);

  tp6 <= t11;

  u10 : g2 port map (
    a => p23,
    b => p25,
    y => t12);


  u11 : inv port map (
    a => p20,
    y => t13);

  p22 <= t13;

  u12 : rsflop port map (
    r => t13,
    s => p24,
    q => t14,
    qb => p28);

  tp5 <= t14;

  u13 : g2 port map (
    a => t14,
    b => p26,
    y => t15);


  u14 : rs4flop port map (
    r => t13,
    s => t12,
    s2 => t11,
    s3 => '1',
    s4 => t15,
    q => t16);

  tp4 <= t16;

  u15 : inv2 port map (
    a => t16,
    y => p8_p9,
    y2 => p6);


  u16 : g5 port map (
    a => p17,
    b => p19,
    c => p21,
    d => p18,
    e => p10,
    y => t18,
    y2 => t19);

  tp3 <= t18;

  u17 : g2 port map (
    a => t18,
    b => p16,
    y => p14);


  u18 : g2 port map (
    a => t19,
    b => p12,
    y => p15);



end gates;

