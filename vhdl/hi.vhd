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
-- HI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hi is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p22 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4_p5_p6 : out logicsig;
      p7_p9 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p17 : out logicsig;
      p18_p19 : out logicsig;
      p20_tp4 : out logicsig;
      p21 : out logicsig;
      p23_p25_p26 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end hi;
architecture gates of hi is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p27,
    b => d,
    y => p24,
    y2 => t1);

  p17 <= d;

  u2 : g2 port map (
    a => t1,
    b => p22,
    y => t2,
    y2 => p23_p25_p26);


  u3 : rsflop port map (
    r => a,
    s => t2,
    q => b);

  tp5 <= b;

  u4 : inv port map (
    a => b,
    y => p21);


  u5 : g3 port map (
    a => p2,
    b => p3,
    c => e,
    y => p1,
    y2 => t4);

  p10 <= e;
  p28 <= t4;

  u6 : g2 port map (
    a => p22,
    b => t4,
    y => t5,
    y2 => p4_p5_p6);


  u7 : rsflop port map (
    r => a,
    s => t5,
    q => c);

  tp1 <= c;

  u8 : inv port map (
    a => c,
    y => p8);


  u9 : inv2 port map (
    a => p13,
    y2 => t7);


  u10 : g2 port map (
    a => b,
    b => t7,
    y => t8);


  u11 : rsflop port map (
    r => p16,
    s => t8,
    q => t9,
    qb => d);

  p20_tp4 <= t9;

  u12 : rsflop port map (
    r => p16,
    s => p15,
    q => tp6,
    qb => t10);


  u13 : g2 port map (
    a => t9,
    b => t10,
    y2 => p18_p19);


  u14 : g2 port map (
    a => t7,
    b => c,
    y => t12);


  u15 : rsflop port map (
    r => p11,
    s => t12,
    q => t13,
    qb => e);

  tp2 <= t13;

  u16 : rsflop port map (
    r => p11,
    s => p12,
    q => tp3,
    qb => t14);


  u17 : g2 port map (
    a => t13,
    b => t14,
    y2 => p7_p9);


  u18 : inv port map (
    a => p14,
    y => a);



end gates;

