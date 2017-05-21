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
-- ND module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nd is
    port (
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7_tp1 : out logicsig;
      p24_tp6 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end nd;
architecture gates of nd is
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
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => h,
    y => t1);

  tp3 <= h;

  u2 : g2 port map (
    a => i,
    b => d,
    y => t2);

  tp2 <= i;

  u3 : g2 port map (
    a => t1,
    b => t2,
    y => p7_tp1);


  u4 : g2 port map (
    a => a,
    b => i,
    y => t3);


  u5 : g2 port map (
    a => h,
    b => d,
    y => t4);


  u6 : g3 port map (
    a => b,
    b => t3,
    c => t4,
    y => p1);


  u7 : g3 port map (
    a => e,
    b => t3,
    c => t4,
    y => p3);


  u8 : rsflop port map (
    r => c,
    s => p11,
    q => h);


  u9 : rsflop port map (
    r => c,
    s => p9,
    q => i);


  u10 : rsflop port map (
    r => c,
    s => p20,
    q => f);

  tp4 <= f;

  u11 : rsflop port map (
    r => c,
    s => p22,
    q => g);

  tp5 <= g;

  u12 : g2 port map (
    a => a,
    b => f,
    y => t10);


  u13 : g2 port map (
    a => g,
    b => d,
    y => t11);


  u14 : g2 port map (
    a => t10,
    b => t11,
    y => p24_tp6);


  u15 : g2 port map (
    a => a,
    b => g,
    y => t12);


  u16 : g2 port map (
    a => f,
    b => d,
    y => t13);


  u17 : g3 port map (
    a => b,
    b => t12,
    c => t13,
    y => p28);


  u18 : g3 port map (
    a => e,
    b => t12,
    c => t13,
    y => p26);


  u19 : inv2 port map (
    a => p13,
    y => a,
    y2 => d);


  u20 : inv2 port map (
    a => p18,
    y => b,
    y2 => e);


  u21 : inv port map (
    a => p16,
    y => c);


  p5 <= '0';

end gates;

