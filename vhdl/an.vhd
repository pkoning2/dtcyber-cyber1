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
-- AN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity an is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p7 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end an;
architecture gates of an is
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

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u2 : r4s4flop port map (
    r => b,
    r2 => c,
    r3 => d,
    r4 => '1',
    s => p8,
    s2 => '1',
    s3 => '1',
    s4 => '1',
    q => t1,
    qb => e);

  p10 <= t1;
  tp1 <= t1;

  u4 : r4s4flop port map (
    r => b,
    r2 => c,
    r3 => d,
    r4 => '1',
    s => p5,
    s2 => '1',
    s3 => '1',
    s4 => '1',
    q => t2,
    qb => f);

  p7 <= t2;
  tp2 <= t2;

  u6 : r4s4flop port map (
    r => b,
    r2 => c,
    r3 => d,
    r4 => '1',
    s => p24,
    s2 => '1',
    s3 => '1',
    s4 => '1',
    q => t3,
    qb => g);

  p21 <= t3;
  tp6 <= t3;

  u8 : r4s4flop port map (
    r => b,
    r2 => c,
    r3 => d,
    r4 => '1',
    s => p22,
    s2 => '1',
    s3 => '1',
    s4 => '1',
    q => t4,
    qb => h);

  p25 <= t4;
  tp5 <= t4;

  u10 : g4 port map (
    a => e,
    b => f,
    c => g,
    d => h,
    y => t5,
    y2 => p27);

  p28 <= t5;
  tp4 <= t5;

  u11 : g4 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p3,
    y => tp3,
    y2 => p1);


  u12 : g4 port map (
    a => p20,
    b => p15,
    c => p23,
    d => p9,
    y2 => t9);

  p18 <= t9;
  p26 <= t9;

  u14 : inv port map (
    a => p14,
    y => b);


  u15 : inv port map (
    a => p12,
    y => c);


  u16 : inv port map (
    a => p16,
    y => d);


  u17 : inv2 port map (
    a => p13,
    y => p11,
    y2 => t10);


  u18 : inv port map (
    a => t10,
    y => t11);

  p17 <= t11;
  p19 <= t11;


end gates;

