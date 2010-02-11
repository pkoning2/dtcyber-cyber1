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
-- JU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ju is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p15 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4_tp2 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p12_tp1 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16_tp3 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20_tp5 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end ju;
architecture gates of ju is
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
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;

begin -- gates
  u1 : rsflop port map (
    r => p7,
    s => p8,
    q => p12_tp1,
    qb => t2);

  p9 <= t2;

  u2 : inv port map (
    a => p3,
    y => t3);


  u3 : g2 port map (
    a => a,
    b => t3,
    y => t4,
    y2 => t5);

  p16_tp3 <= t4;
  tp4 <= t5;

  u4 : g2 port map (
    a => p5,
    b => t4,
    y => t6);

  p19 <= t6;

  u5 : g4 port map (
    a => t2,
    b => p8,
    c => p6,
    d => t6,
    y => p10);


  u6 : g2 port map (
    a => t6,
    b => p2,
    y => p4_tp2,
    y2 => p1);


  u7 : g2 port map (
    a => p22,
    b => t5,
    y => p20_tp5);


  u8 : inv2 port map (
    a => t5,
    y => p18,
    y2 => p17);


  u9 : inv port map (
    a => t3,
    y => t9);


  u10 : g2 port map (
    a => p11,
    b => t9,
    y2 => p14);


  u11 : g3 port map (
    a => t9,
    b => p15,
    c => a,
    y => p13);


  u12 : inv port map (
    a => p24,
    y => t10);


  u13 : inv port map (
    a => p21,
    y => t11);


  u14 : rsflop port map (
    r => t11,
    s => t10,
    q => tp6,
    qb => a);


  u15 : inv port map (
    a => p26,
    y => t12);


  u16 : inv port map (
    a => p27,
    y => t13);


  u17 : g2 port map (
    a => t12,
    b => p23,
    y => p25);


  u18 : g2 port map (
    a => p23,
    b => t13,
    y => p28);



end gates;

