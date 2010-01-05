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
-- SL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity sl is
    port (
      p6 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  coaxsig;
      p19 : in  logicsig;
      p20 : in  coaxsig;
      p25 : in  coaxsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig);

end sl;
architecture gates of sl is
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

  signal a : logicsig;
  signal b : logicsig;
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
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;
  signal v : logicsig;
  signal w : logicsig;
  signal x : logicsig;
  signal y : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p25,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => t2);

  tp4 <= t2;

  u3 : inv port map (
    a => t2,
    y => t3);


  u4 : inv port map (
    a => x,
    y => t4);

  p10 <= x;
  tp1 <= x;

  u5 : g2 port map (
    a => w,
    b => t4,
    y => t5);


  u6 : g3 port map (
    a => p6,
    b => t3,
    c => t5,
    y2 => t6);

  p2 <= t6;
  p4 <= t6;
  p8 <= t6;

  u7 : inv port map (
    a => t6,
    y => t7);

  p1 <= t7;
  p3 <= t7;
  p5 <= t7;

  u8 : cxreceiver port map (
    a => p20,
    y => t8);


  u9 : rsflop port map (
    r => a,
    s => t8,
    q => t9);


  u10 : inv port map (
    a => t9,
    y => t10);


  u11 : inv port map (
    a => y,
    y => t11);

  tp5 <= y;

  u12 : g2 port map (
    a => t10,
    b => t11,
    y => w);


  u13 : inv2 port map (
    a => p13,
    y2 => v);

  p9 <= v;

  u14 : g2 port map (
    a => w,
    b => v,
    y => t13);


  u15 : rsflop port map (
    r => b,
    s => t13,
    q => x);


  u16 : cxreceiver port map (
    a => p18,
    y => t14);


  u17 : rsflop port map (
    r => a,
    s => t14,
    q => y);


  u18 : g2 port map (
    a => y,
    b => v,
    y => t15);


  u19 : rsflop port map (
    r => b,
    s => t15,
    q => t16);

  tp3 <= t16;

  u20 : inv port map (
    a => t16,
    y => p7);


  u21 : g4 port map (
    a => p14,
    b => p15,
    c => p16,
    d => p17,
    y => p11,
    y2 => t17);


  u22 : g2 port map (
    a => p12,
    b => t17,
    y => b);


  u23 : inv port map (
    a => p19,
    y => a);



end gates;

