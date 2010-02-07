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
-- HA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ha is
    port (
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig);

end ha;
architecture gates of ha is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p20,
    y => i);


  u2 : g2 port map (
    a => p17,
    b => p11,
    y => j);


  u3 : inv port map (
    a => p18,
    y => c);


  u4 : g2 port map (
    a => g,
    b => i,
    y => t1);


  u5 : g2 port map (
    a => i,
    b => h,
    y => t2);


  u6 : rsflop port map (
    r => t2,
    s => t1,
    q => t3,
    qb => t4);

  tp1 <= t3;

  u7 : g4 port map (
    a => j,
    b => a,
    c => c,
    d => t4,
    y => t5);

  tp4 <= a;

  u8 : g3 port map (
    a => t3,
    b => a,
    c => j,
    y => t6);


  u10 : r4s4flop port map (
    r => t6,
    r2 => p8,
    r3 => c,
    r4 => '1',
    s => t5,
    s2 => p7,
    s3 => '1',
    s4 => '1',
    q => g,
    qb => h);


  u11 : g2 port map (
    a => e,
    b => i,
    y => t7);


  u12 : g2 port map (
    a => i,
    b => f,
    y => t8);


  u13 : rsflop port map (
    r => t8,
    s => t7,
    q => a,
    qb => t10);


  u14 : g3 port map (
    a => j,
    b => c,
    c => t10,
    y => t11);


  u15 : g2 port map (
    a => a,
    b => j,
    y => t12);


  u16 : r4s4flop port map (
    r => t12,
    r2 => p19,
    r3 => c,
    r4 => '1',
    s => t11,
    s2 => p22,
    s3 => '1',
    s4 => '1',
    q => e,
    qb => f);


  u17 : g2 port map (
    a => g,
    b => f,
    y => tp2,
    y2 => t13);

  p3 <= t13;
  p6 <= t13;

  u18 : g2 port map (
    a => h,
    b => f,
    y => tp5,
    y2 => t14);

  p21 <= t14;
  p24 <= t14;

  u19 : g2 port map (
    a => g,
    b => e,
    y => tp3,
    y2 => t15);

  p1 <= t15;
  p4 <= t15;

  u21 : g2 port map (
    a => h,
    b => e,
    y => tp6,
    y2 => t16);

  p23 <= t16;
  p26 <= t16;


end gates;

