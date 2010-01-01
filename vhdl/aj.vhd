-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- AJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity aj is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p27 : out logicsig);

end aj;
architecture gates of aj is
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
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
  signal x : logicsig;
  signal y : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => d,
    y => t1);

  tp2 <= a;

  u2 : g2 port map (
    a => p1,
    b => p3,
    y => t2);


  u3 : g2 port map (
    a => f,
    b => t2,
    y => t3);


  u4 : r4s4flop port map (
    r => e,
    r2 => c,
    r3 => '1',
    r4 => '1',
    s => t1,
    s2 => t3,
    s3 => '1',
    s4 => '1',
    q => t4);

  p4 <= t4;
  tp1 <= t4;

  u5 : g2 port map (
    a => t4,
    b => p10,
    y => p6);


  u6 : g2 port map (
    a => b,
    b => d,
    y => x);

  tp5 <= b;

  u7 : g2 port map (
    a => p28,
    b => p26,
    y => y);


  u8 : g2 port map (
    a => f,
    b => y,
    y => t7);


  u9 : r4s4flop port map (
    r => c,
    r2 => e,
    r3 => '1',
    r4 => '1',
    s => x,
    s2 => t7,
    s3 => '1',
    s4 => '1',
    q => t8,
    qb => p23);

  p21 <= t8;
  tp6 <= t8;

  u10 : g2 port map (
    a => p24,
    b => t8,
    y => p27);


  u11 : rs2flop port map (
    r => t11,
    s => p11,
    s2 => p9,
    q => a);


  u13 : inv2 port map (
    a => p8,
    y => t11,
    y2 => t12);

  p13 <= t12;
  p15 <= t12;
  p17 <= t12;
  p19 <= t12;

  u14 : rs2flop port map (
    r => t11,
    s => p20,
    s2 => p18,
    q => b);


  u15 : inv2 port map (
    a => p7,
    y => c,
    y2 => d);


  u16 : inv2 port map (
    a => p22,
    y => e,
    y2 => f);

  u17 : r4s4flop port map (
    r  => c,
    r2 => e,
    r3 => '1',
    r4 => '1',
    s  => x,
    s2 => y,
    s3 => '1',
    s4 => '1',
    q  => t13);

  p14 <= t13;

  u18 : inv port map (
    a => t13,
    y => p5);

end gates;

