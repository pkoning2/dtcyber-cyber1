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
-- MT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mt is
    port (
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  coaxsig;
      p11 : in  coaxsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp4 : out logicsig;
      p1_tp3 : out coaxsig;
      p2_tp1 : out coaxsig;
      p3_tp2 : out coaxsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p15_tp6 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p22 : out logicsig;
      p27 : out logicsig;
      p28_tp5 : out logicsig);

end mt;
architecture gates of mt is
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

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : g4 port map (
    a => p26,
    b => p25,
    c => p24,
    d => p23,
    y => t1,
    y2 => t2);

  p28_tp5 <= t1;

  u2 : g2 port map (
    a => t1,
    b => d,
    y => p27);

  tp4 <= d;

  u3 : g2 port map (
    a => t2,
    b => b,
    y => p22);


  u4 : g4 port map (
    a => p21,
    b => p19,
    c => p20,
    d => p17,
    y => t3,
    y2 => t4);

  p15_tp6 <= t3;

  u5 : g2 port map (
    a => t3,
    b => d,
    y => p18);


  u6 : g2 port map (
    a => t4,
    b => b,
    y => p16);


  u7 : g2 port map (
    a => p4,
    b => c,
    y => t5);


  u8 : cxdriver port map (
    a => t5,
    y => p2_tp1);


  u9 : g2 port map (
    a => p6,
    b => c,
    y => t6);


  u10 : cxdriver port map (
    a => t6,
    y => p3_tp2);


  u11 : g2 port map (
    a => p5,
    b => c,
    y => t7);


  u12 : cxdriver port map (
    a => t7,
    y => p1_tp3);


  u13 : inv port map (
    a => p12,
    y => d);


  u14 : inv port map (
    a => p14,
    y => b);


  u15 : inv2 port map (
    a => p8,
    y => c,
    y2 => e);


  u16 : cxreceiver port map (
    a => p10,
    y => t8);


  u17 : rsflop port map (
    r => e,
    s => t8,
    q => p7);


  u18 : cxreceiver port map (
    a => p11,
    y => t9);


  u19 : rsflop port map (
    r => e,
    s => t9,
    q => p9);



end gates;

