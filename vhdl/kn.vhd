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
-- KN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kn is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  coaxsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  coaxsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      p2 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13_tp2 : out logicsig;
      p15_p17 : out logicsig;
      p16_tp3 : out logicsig;
      p21_tp4 : out logicsig;
      p22_tp5 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end kn;
architecture gates of kn is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal h : logicsig;
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

begin -- gates
  u1 : cxreceiver port map (
    a => p5,
    y => t1);


  u2 : rsflop port map (
    r => d,
    s => t1,
    qb => t2);


  u3 : g2 port map (
    a => t2,
    b => p6,
    y => p8);


  u4 : cxreceiver port map (
    a => p9,
    y => t3);


  u5 : rsflop port map (
    r => b,
    s => t3,
    qb => t4);


  u6 : g2 port map (
    a => e,
    b => t4,
    y => p13_tp2);


  u7 : g2 port map (
    a => t4,
    b => p12,
    y2 => p11);


  u8 : inv port map (
    a => p7,
    y => t5);


  u9 : g2 port map (
    a => p6,
    b => t5,
    y => t6);


  u10 : rsflop port map (
    r => d,
    s => t6,
    q => tp1,
    qb => p10);


  u11 : g2 port map (
    a => h,
    b => p20,
    y => t7);

  p15_p17 <= h;

  u12 : inv port map (
    a => h,
    y => t8);


  u13 : g2 port map (
    a => p18,
    b => t8,
    y => t9);


  u14 : g2 port map (
    a => t7,
    b => t9,
    y => p16_tp3);


  u15 : inv port map (
    a => p19,
    y => t10);


  u16 : g2 port map (
    a => t10,
    b => p24,
    y => p22_tp5,
    y2 => h);


  u17 : g2 port map (
    a => p3,
    b => p4,
    y => d);


  u18 : inv port map (
    a => p1,
    y => t11);


  u19 : g2 port map (
    a => t11,
    b => p4,
    y => p2,
    y2 => t12);


  u20 : inv port map (
    a => t12,
    y => b);


  u21 : rsflop port map (
    r => b,
    s => p14,
    q => c,
    qb => e);


  u22 : inv port map (
    a => p24,
    y => t13);


  u23 : g2 port map (
    a => c,
    b => p23,
    y => t14,
    y2 => p28);

  p25 <= t14;

  u24 : g2 port map (
    a => t13,
    b => t14,
    y => p26,
    y2 => p21_tp4);


  u25 : inv port map (
    a => p23,
    y => t15);


  u26 : g2 port map (
    a => t15,
    b => c,
    y2 => p27);



end gates;

