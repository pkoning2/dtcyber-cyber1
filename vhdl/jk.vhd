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
-- JK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jk is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p15 : in  coaxsig;
      p17 : in  coaxsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out coaxsig;
      p4 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig);

end jk;
architecture gates of jk is
  component cxdriver
    port (
      a : in  logicsig;
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

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
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
  signal t17 : logicsig;
  signal t18 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p15,
    y => t1);


  u2 : inv port map (
    a => p23,
    y => t2);


  u3 : inv2 port map (
    a => t2,
    y => p14,
    y2 => t3);


  u4 : rsflop port map (
    r => t3,
    s => t1,
    q => t4);

  tp5 <= t4;

  u5 : inv port map (
    a => t4,
    y => p26);


  u6 : g2 port map (
    a => t4,
    b => p24,
    y => t5);


  u7 : inv port map (
    a => p6,
    y => t6);

  p4 <= t6;

  u8 : g2 port map (
    a => a,
    b => t6,
    y => t7);

  p7 <= t7;
  p8 <= a;

  u9 : r2sflop port map (
    r => t7,
    r2 => p1,
    s => t5,
    q => t8);

  tp3 <= t8;

  u10 : inv port map (
    a => p10,
    y => t9);


  u11 : g2 port map (
    a => t8,
    b => t9,
    y2 => p13);


  u12 : inv port map (
    a => t8,
    y => t10);


  u13 : g2 port map (
    a => t4,
    b => t10,
    y => p19,
    y2 => t11);


  u14 : g2 port map (
    a => t11,
    b => p18,
    y => t12);


  u15 : r2sflop port map (
    r => p1,
    r2 => t7,
    s => t12,
    q => t13);

  tp4 <= t13;

  u16 : inv port map (
    a => t13,
    y => p21);


  u17 : g2 port map (
    a => t6,
    b => p3,
    y => t14);


  u18 : cxdriver port map (
    a => t14,
    y => p2);


  u19 : cxreceiver port map (
    a => p17,
    y => t15);


  u20 : rsflop port map (
    r => p22,
    s => t15,
    q => t16);

  p25 <= t16;
  tp6 <= t16;

  u21 : inv port map (
    a => t16,
    y => t17);


  u22 : g2 port map (
    a => t17,
    b => p11,
    y2 => p12);


  u23 : inv2 port map (
    a => p5,
    y2 => a);


  u24 : g2 port map (
    a => p9,
    b => p20,
    y2 => t18);

  p16 <= t18;

  u25 : inv port map (
    a => t18,
    y => p27);



end gates;

