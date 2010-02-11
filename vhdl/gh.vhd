-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- GH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gh is
    port (
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p1_p3_p5_p23 : out logicsig;
      p6_tp1 : out logicsig;
      p17_tp5 : out logicsig;
      p19_tp6 : out logicsig);

end gh;
architecture gates of gh is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal k : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t20 : logicsig;
  signal t21 : logicsig;
  signal t22 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => k,
    y => t1,
    y2 => t2);


  u2 : g2 port map (
    a => b,
    b => t2,
    y => t3);


  u3 : g2 port map (
    a => t1,
    b => a,
    y => t4);

  tp3 <= a;

  u4 : g2 port map (
    a => t3,
    b => t4,
    y => p6_tp1);


  u5 : g2 port map (
    a => a,
    b => c,
    y => t6,
    y2 => t7);

  tp4 <= c;

  u6 : g2 port map (
    a => e,
    b => t7,
    y => t8);

  tp2 <= e;

  u7 : g2 port map (
    a => f,
    b => t6,
    y => t9);


  u8 : g2 port map (
    a => t8,
    b => t9,
    y => p17_tp5);


  u9 : inv2 port map (
    a => a,
    y => t11,
    y2 => t12);


  u10 : g2 port map (
    a => c,
    b => t12,
    y => t13);


  u11 : g2 port map (
    a => d,
    b => t11,
    y => t14);


  u12 : g2 port map (
    a => t13,
    b => t14,
    y => p19_tp6);


  u13 : inv port map (
    a => p8,
    y => t20);


  u14 : inv2 port map (
    a => t20,
    y => e,
    y2 => f);


  u15 : inv port map (
    a => p12,
    y => t21);


  u16 : inv2 port map (
    a => t21,
    y => a,
    y2 => b);


  u17 : inv port map (
    a => p10,
    y => t22);


  u18 : inv2 port map (
    a => t22,
    y => c,
    y2 => d);


  u19 : g3 port map (
    a => a,
    b => c,
    c => e,
    y2 => p1_p3_p5_p23);


  u20 : g4 port map (
    a => p26,
    b => p24,
    c => p28,
    d => p25,
    y2 => k);



end gates;

