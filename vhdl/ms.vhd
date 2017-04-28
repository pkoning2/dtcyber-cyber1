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
-- MS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ms is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  coaxsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      p1_tp2 : out coaxsig;
      p3 : out logicsig;
      p7 : out logicsig;
      p14_tp3 : out logicsig;
      p27_p25_p23_p21_p19_p28_p26_p24_p22_p20_p13_p9_p11 : out logicsig);

end ms;
architecture gates of ms is
  component cxdriver4
    port (
      a : in  logicsig;
      a2 : in  logicsig;
      a3 : in  logicsig;
      a4 : in  logicsig;
      y : out coaxsig);

  end component;

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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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

  signal e : logicsig;
  signal g : logicsig;
  signal t : logicsig;
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
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p2,
    y => t);

  tp1 <= t;

  u2 : inv port map (
    a => p8,
    y => t1);


  u3 : inv port map (
    a => p6,
    y => t2);


  u4 : inv port map (
    a => p4,
    y => t3);


  u5 : g4 port map (
    a => t1,
    b => t2,
    c => t3,
    d => p5,
    y => p3,
    y2 => p7);


  u6 : inv port map (
    a => p5,
    y => t8);


  u7 : g2 port map (
    a => p8,
    b => t,
    y => t4);


  u8 : g2 port map (
    a => p6,
    b => t,
    y => t5);


  u9 : g2 port map (
    a => p4,
    b => t,
    y => t6);


  u10 : g2 port map (
    a => t8,
    b => t,
    y => t7);


  u11 : cxdriver4 port map (
    a => t4,
    a2 => t5,
    a3 => t6,
    a4 => t7,
    y => p1_tp2);


  u12 : cxreceiver port map (
    a => p15,
    y => t9);


  u13 : rsflop port map (
    r => p18,
    s => t9,
    q => t10);


  u14 : inv2 port map (
    a => t10,
    y => e,
    y2 => t11);

  p27_p25_p23_p21_p19_p28_p26_p24_p22_p20_p13_p9_p11 <= t11;

  u15 : inv2 port map (
    a => p16,
    y => tp5,
    y2 => g);


  u16 : inv port map (
    a => p10,
    y => t14);


  u17 : g3 port map (
    a => t11,
    b => g,
    c => t14,
    y => t15);


  u18 : g3 port map (
    a => p10,
    b => g,
    c => e,
    y => t17);


  u19 : g2 port map (
    a => t15,
    b => t17,
    y => t16);


  u20 : g2 port map (
    a => p12,
    b => t16,
    y => p14_tp3);



end gates;

