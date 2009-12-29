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
-- AI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity AI is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig);

end AI;
architecture gates of AI is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
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

  signal '0' : None;
  signal 10 : logicsig;
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
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p28,
    y => b);


  u2 : inv port map (
    a => p2,
    y => a);

  p3 <= a;

  u3 : rsflop port map (
    r => a,
    s => p6,
    q => c);

  tp1 <= c;

  u4 : g2 port map (
    a => c,
    b => b,
    y => p14);


  u5 : rsflop port map (
    r => a,
    s => p8,
    q => d);

  tp2 <= d;

  u6 : g2 port map (
    a => d,
    b => b,
    y => p12);


  u7 : rsflop port map (
    r => a,
    s => p27,
    q => e);

  tp5 <= e;

  u8 : g2 port map (
    a => e,
    b => b,
    y => p17);


  u9 : rsflop port map (
    r => a,
    s => p25,
    q => f);

  tp6 <= f;

  u10 : g2 port map (
    a => f,
    b => b,
    y => p19);


  u11 : g2 port map (
    a => d,
    b => p4,
    y => t1);


  u12 : g2 port map (
    a => p26,
    b => e,
    y => t2);


  u13 : g2 port map (
    a => f,
    b => p9,
    y => t3);


  u14 : g2 port map (
    a => p7,
    b => c,
    y => t4);


  u15 : inv port map (
    a => p10,
    y => t5);


  u16 : g5 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => t5,
    y => t6);

  p11 <= t6;
  tp3 <= t6;

  u17 : g2 port map (
    a => t6,
    b => p21,
    y => p15);


  u18 : g2 port map (
    a => c,
    b => p4,
    y => t7);


  u19 : g2 port map (
    a => p26,
    b => d,
    y => t8);


  u20 : g2 port map (
    a => e,
    b => p9,
    y => t9);


  u21 : g2 port map (
    a => p7,
    b => f,
    y => t10);


  u22 : inv port map (
    a => p23,
    y => t11);


  u23 : g5 port map (
    a => t7,
    b => t8,
    c => t9,
    d => 10,
    e => t11,
    y => t12);

  p20 <= t12;
  tp4 <= t12;

  u24 : g2 port map (
    a => t12,
    b => p22,
    y => p16);


  p5 <= '0';
  p18 <= '0';

end gates;

