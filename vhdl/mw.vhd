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
-- MW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mw is
    port (
      p2 : in  coaxsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p14 : in  logicsig;
      p19 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  coaxsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p8_p12 : out logicsig;
      p13_p11 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p20_p18 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p28_p25 : out logicsig);

end mw;
architecture gates of mw is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
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
    a => p26,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => b,
    qb => t2);

  tp6 <= b;

  u3 : inv port map (
    a => t2,
    y => d);

  p28_p25 <= d;

  u4 : g2 port map (
    a => b,
    b => p10,
    y => p15,
    y2 => p17);


  u5 : cxreceiver port map (
    a => p2,
    y => t3);


  u6 : rsflop port map (
    r => a,
    s => t3,
    q => c,
    qb => t4);

  tp1 <= c;

  u7 : inv port map (
    a => t4,
    y => e);

  p8_p12 <= e;

  u8 : g2 port map (
    a => c,
    b => p10,
    y => p3,
    y2 => p1);


  u9 : g3 port map (
    a => p14,
    b => d,
    c => p23,
    y => t10);

  tp5 <= t10;

  u10 : g3 port map (
    a => p4,
    b => e,
    c => p5,
    y => t11);

  tp2 <= t11;

  u11 : g2 port map (
    a => t10,
    b => t11,
    y => p22);


  u12 : g3 port map (
    a => p23,
    b => b,
    c => p24,
    y => t12);

  tp4 <= t12;

  u13 : g3 port map (
    a => p5,
    b => p7,
    c => c,
    y => t13);

  tp3 <= t13;

  u14 : g2 port map (
    a => t12,
    b => t13,
    y => p21);


  u15 : g3 port map (
    a => p24,
    b => b,
    c => p19,
    y => t14);


  u16 : g3 port map (
    a => p7,
    b => c,
    c => p6,
    y => t15);


  u17 : g2 port map (
    a => t14,
    b => t15,
    y => p20_p18);


  u18 : g3 port map (
    a => p19,
    b => b,
    c => p14,
    y => t16);


  u19 : g3 port map (
    a => p6,
    b => c,
    c => p4,
    y => t17);


  u20 : g2 port map (
    a => t16,
    b => t17,
    y => p13_p11);


  u21 : inv port map (
    a => p27,
    y => a);



end gates;

