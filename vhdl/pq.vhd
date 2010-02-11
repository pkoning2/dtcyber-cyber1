-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- PQ module, rev D -- deadstart panel control and central busy state
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pq is
    port (
      p1 : in  coaxsig;
      p2 : in  logicsig := '0';
      p3 : in  logicsig;
      p4 : in  logicsig := '0';
      p5 : in  coaxsig;
      p6 : in  logicsig;
      p11 : in  logicsig;
      p16 : in  logicsig := '0';
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  coaxsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p13_p27 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig);

end pq;
architecture gates of pq is
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

  signal a : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal k : logicsig;
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

begin -- gates
  u1 : inv2 port map (
    a => p6,
    y2 => t1);


  u2 : g2 port map (
    a => t13,
    b => t1,
    y => t2);


  u3 : g2 port map (
    a => p23,
    b => t1,
    y => t3);


  u4 : g2 port map (
    a => p25,
    b => t1,
    y => t4);


  u5 : cxreceiver port map (
    a => p1,
    y => t5);


  u6 : cxreceiver port map (
    a => p5,
    y => t6);


  u7 : cxreceiver port map (
    a => p21,
    y => t7);


  u8 : r4s4flop port map (
    r => t5,
    r2 => t6,
    r3 => t7,
    r4 => p3,
    s => t2,
    s2 => t3,
    s3 => t4,
    s4 => '1',
    q => t8);

  tp1 <= t8;

  u9 : inv2 port map (
    a => t8,
    y2 => t9);

  p13_p27 <= t9;

  u10 : inv port map (
    a => t9,
    y => t10);


  u11 : g4 port map (
    a => p22,
    b => p24,
    c => p26,
    d => p28,
    y2 => a);

  tp5 <= a;

  u12 : inv2 port map (
    a => p11,
    y => h,
    y2 => g);


  u13 : inv port map (
    a => p17,
    y => j);


  u14 : inv port map (
    a => p19,
    y => k);


  u15 : g4 port map (
    a => g,
    b => a,
    c => j,
    d => k,
    y2 => t11);

  tp6 <= t11;

  u16 : inv port map (
    a => t11,
    y => p10);


  u17 : g2 port map (
    a => t10,
    b => t11,
    y2 => t12);

  p18 <= t12;

  u18 : inv2 port map (
    a => t12,
    y => p20,
    y2 => t13);


  u19 : g4 port map (
    a => a,
    b => h,
    c => j,
    d => k,
    y => p14,
    y2 => t14);

  tp2 <= t14;

  u20 : inv port map (
    a => t14,
    y => p8);


  u21 : inv port map (
    a => p4,
    y => t15);

  p9 <= t15;

  u22 : inv port map (
    a => p2,
    y => t16);

  p7 <= t16;

  u23 : g2 port map (
    a => t15,
    b => t16,
    y => p12);


  p15 <= p16;

end gates;

