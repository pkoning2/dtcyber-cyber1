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
-- CF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cf is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p18 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end cf;
architecture gates of cf is
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
  signal d : logicsig;
  signal k : logicsig;
  signal s : logicsig;
  signal t : logicsig;
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
  u1 : inv port map (
    a => p14,
    y => b);


  u2 : inv port map (
    a => p17,
    y => c);


  u3 : inv port map (
    a => b,
    y => t1);


  u4 : g2 port map (
    a => p15,
    b => t1,
    y => a);


  u5 : inv port map (
    a => c,
    y => t2);


  u6 : g2 port map (
    a => p15,
    b => t2,
    y => d);


  u7 : g2 port map (
    a => p13,
    b => p16,
    y => t3);

  p9 <= t3;

  u8 : g2 port map (
    a => t1,
    b => t3,
    y => t4);


  u9 : rsflop port map (
    r => p12,
    s => t4,
    q => t5);

  tp1 <= t5;

  u10 : g2 port map (
    a => t5,
    b => s,
    y => p8);

  tp4 <= s;

  u11 : g2 port map (
    a => t2,
    b => t3,
    y => t6);


  u12 : rsflop port map (
    r => p10,
    s => t6,
    q => t7);


  u13 : g2 port map (
    a => t7,
    b => t,
    y => p6);

  tp6 <= t;

  u14 : inv2 port map (
    a => p25,
    y2 => p24);


  u15 : g2 port map (
    a => p25,
    b => p23,
    y => k);


  u16 : inv2 port map (
    a => p27,
    y2 => p28);


  u17 : g2 port map (
    a => p23,
    b => p27,
    y => t8);


  u18 : r2sflop port map (
    r => t8,
    r2 => b,
    s => k,
    q => s);


  u19 : rsflop port map (
    r => b,
    s => a,
    q => t9);

  tp3 <= t9;

  u20 : g2 port map (
    a => s,
    b => t9,
    y => p21);


  u21 : r2sflop port map (
    r => k,
    r2 => c,
    s => t8,
    q => t);


  u22 : rsflop port map (
    r => c,
    s => d,
    q => t10);

  tp5 <= t10;

  u23 : g2 port map (
    a => t,
    b => t10,
    y => p18);


  u24 : g2 port map (
    a => p3,
    b => p5,
    y => p1,
    y2 => p4);



end gates;

