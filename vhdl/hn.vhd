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
-- HN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hn is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p21 : in  logicsig;
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
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end hn;
architecture gates of hn is
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
  signal b : logicsig;
  signal c : logicsig;
  signal g2 : logicsig;
  signal n : logicsig;
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

begin -- gates
  u1 : g2 port map (
    a => p5,
    b => b,
    y => t1,
    y2 => p2);

  p1 <= t1;

  u2 : rsflop port map (
    r => a,
    s => t1,
    q => t2,
    qb => p19);

  tp1 <= t2;

  u3 : inv port map (
    a => t2,
    y => p8);


  u4 : g2 port map (
    a => g2,
    b => c,
    y => p3,
    y2 => p4);

  tp4 <= c;

  u5 : g2 port map (
    a => p10,
    b => c,
    y => t3,
    y2 => p15);

  p14 <= t3;

  u6 : rsflop port map (
    r => a,
    s => t3,
    q => t4);

  tp3 <= t4;

  u7 : g2 port map (
    a => t4,
    b => c,
    y => p12,
    y2 => p13);


  u8 : g2 port map (
    a => p24,
    b => b,
    y => t5,
    y2 => p27);

  p28 <= t5;

  u9 : rsflop port map (
    r => a,
    s => t5,
    q => t6);

  tp6 <= t6;

  u10 : inv2 port map (
    a => t6,
    y2 => p18);


  u11 : g2 port map (
    a => t6,
    b => c,
    y => p26,
    y2 => p25);


  u12 : g2 port map (
    a => p21,
    b => b,
    y => t7,
    y2 => p16);

  p17 <= t7;

  u13 : r2sflop port map (
    r => p9,
    r2 => a,
    s => t7,
    q => t8);

  tp5 <= t8;

  u14 : inv2 port map (
    a => t8,
    y2 => n);

  p6 <= n;

  u15 : g2 port map (
    a => p7,
    b => n,
    y => t9);


  u16 : rsflop port map (
    r => t9,
    s => p11,
    q => t10);

  tp2 <= t10;

  u17 : g2 port map (
    a => p22,
    b => t10,
    y => a,
    y2 => b);


  u18 : g2 port map (
    a => t10,
    b => p23,
    y => p20,
    y2 => c);



end gates;

