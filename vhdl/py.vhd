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
-- PY module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity py is
    port (
      p4 : in  coaxsig;
      p10 : in  coaxsig;
      p13 : in  coaxsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  coaxsig;
      p17 : in  coaxsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  coaxsig;
      p25 : in  logicsig;
      p26 : in  coaxsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p2 : out logicsig;
      p5_p8 : out logicsig;
      p6 : out logicsig;
      p7_p18 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end py;
architecture gates of py is
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

  component r4s4flop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      r3 : in  logicsig;
      r4 : in  logicsig;
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

  signal a : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p4,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => t2);

  tp1 <= t2;

  u3 : inv2 port map (
    a => t2,
    y => p6,
    y2 => p1_p2);


  u4 : cxreceiver port map (
    a => p16,
    y => t4);


  u5 : rsflop port map (
    r => a,
    s => t4,
    q => t5);

  tp2 <= t5;

  u6 : inv2 port map (
    a => t5,
    y => p9,
    y2 => p7_p18);


  u7 : cxreceiver port map (
    a => p10,
    y => t7);


  u8 : rsflop port map (
    r => a,
    s => t7,
    q => t8);

  tp3 <= t8;

  u9 : inv2 port map (
    a => t8,
    y => p12,
    y2 => p5_p8);


  u10 : cxreceiver port map (
    a => p26,
    y => t10);


  u11 : rsflop port map (
    r => p25,
    s => t10,
    q => t11);

  tp5 <= t11;

  u12 : inv2 port map (
    a => t11,
    y => p27,
    y2 => p28);


  u13 : cxreceiver port map (
    a => p17,
    y => t12);


  u14 : cxreceiver port map (
    a => p13,
    y => t13);


  u15 : r4s4flop port map (
    r => p19,
    r2 => t13,
    r3 => '1',
    r4 => '1',
    s => p15,
    s2 => t12,
    s3 => '1',
    s4 => '1',
    q => t14);

  tp4 <= t14;

  u16 : inv port map (
    a => t14,
    y => p11);


  u17 : cxreceiver port map (
    a => p24,
    y => t15);


  u18 : rsflop port map (
    r => p23,
    s => t15,
    q => t16);

  tp6 <= t16;

  u19 : inv2 port map (
    a => t16,
    y => p21,
    y2 => p22);


  u20 : g2 port map (
    a => p14,
    b => p20,
    y2 => t17);


  u21 : inv port map (
    a => t17,
    y => a);



end gates;

