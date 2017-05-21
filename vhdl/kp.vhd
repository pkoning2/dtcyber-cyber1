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
-- KP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kp is
    port (
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  coaxsig;
      p12 : in  logicsig;
      p13 : in  coaxsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5_tp2 : out logicsig;
      p6_tp1 : out logicsig;
      p14_tp6 : out logicsig;
      p16 : out logicsig;
      p19_tp5 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end kp;
architecture gates of kp is
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
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t20 : logicsig;
  signal t21 : logicsig;
  signal t22 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p13,
    y => t1);


  u2 : rsflop port map (
    r => c,
    s => t1,
    q => t2,
    qb => t3);

  tp4 <= t2;

  u3 : g2 port map (
    a => a,
    b => t2,
    y => t4);


  u4 : g2 port map (
    a => b,
    b => t3,
    y => t5);


  u5 : g2 port map (
    a => t4,
    b => t5,
    y => t6);


  u6 : g2 port map (
    a => t6,
    b => f,
    y => p6_tp1);


  u7 : g2 port map (
    a => t6,
    b => e,
    y => p4);


  u8 : g2 port map (
    a => t6,
    b => d,
    y => p2);


  u9 : cxreceiver port map (
    a => p11,
    y => t10);


  u10 : rsflop port map (
    r => c,
    s => t10,
    q => t11,
    qb => t12);

  tp3 <= t11;

  u11 : g2 port map (
    a => t11,
    b => a,
    y => t13);


  u12 : g2 port map (
    a => t12,
    b => b,
    y => t14);


  u13 : g2 port map (
    a => t13,
    b => t14,
    y => t15);


  u14 : g2 port map (
    a => t15,
    b => f,
    y => p5_tp2);


  u15 : g2 port map (
    a => t15,
    b => e,
    y => p3);


  u16 : g2 port map (
    a => t15,
    b => d,
    y => p1);


  u17 : inv port map (
    a => p24,
    y => t20);

  p26 <= t20;

  u18 : g2 port map (
    a => p27,
    b => t20,
    y => p25,
    y2 => p28);


  u19 : inv port map (
    a => p23,
    y => t21);

  p21 <= t21;

  u20 : g2 port map (
    a => p20,
    b => t21,
    y => p22,
    y2 => p19_tp5);


  u21 : g2 port map (
    a => p18,
    b => p17,
    y => t22);

  p16 <= t22;

  u22 : g2 port map (
    a => p12,
    b => t22,
    y => p14_tp6);


  u23 : inv2 port map (
    a => p10,
    y => a,
    y2 => b);


  u24 : inv port map (
    a => p15,
    y => c);


  u25 : inv port map (
    a => p7,
    y => d);


  u26 : inv port map (
    a => p8,
    y => e);


  u27 : inv port map (
    a => p9,
    y => f);



end gates;

