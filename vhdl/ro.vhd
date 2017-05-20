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
-- RO module (variant of RL)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ro is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p8 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p22_p23 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ro;
architecture gates of ro is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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
  signal g : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t16 : logicsig;
  signal t20 : logicsig;
  signal t21 : logicsig;
  signal t22 : logicsig;
  signal t23 : logicsig;
  signal t24 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p14,
    b => p7,
    y => c);


  u2 : inv2 port map (
    a => p14,
    y2 => a);

  tp2 <= a;

  u3 : g2 port map (
    a => a,
    b => p27,
    y => d);


  u4 : g2 port map (
    a => p2,
    b => a,
    y => e);


  u5 : g2 port map (
    a => p6,
    b => a,
    y => f);


  u7 : g2 port map (
    a => p24,
    b => a,
    y => g);


  u8 : inv2 port map (
    a => p10,
    y2 => b);

  tp1 <= b;

  u9 : inv port map (
    a => p7,
    y => t1);

  p5 <= t1;

  u10 : inv port map (
    a => p27,
    y => t2);

  p28 <= t2;

  u11 : inv port map (
    a => p2,
    y => t3);


  u12 : inv port map (
    a => p6,
    y => t4);


  u14 : inv port map (
    a => p24,
    y => t6);

  tp6 <= t6;

  u15 : g2 port map (
    a => t1,
    b => p10,
    y => t11);


  u16 : g2 port map (
    a => t11,
    b => c,
    y => p8);


  u17 : g2 port map (
    a => t2,
    b => b,
    y => t12);


  u18 : g2 port map (
    a => t12,
    b => d,
    y => p26);


  u19 : g2 port map (
    a => t3,
    b => b,
    y => t13);


  u20 : g2 port map (
    a => t13,
    b => e,
    y => p1);


  u21 : g2 port map (
    a => t4,
    b => b,
    y => t14);


  u22 : g2 port map (
    a => t14,
    b => f,
    y => p3);


  u25 : g2 port map (
    a => t6,
    b => b,
    y => t16);


  u26 : g2 port map (
    a => t16,
    b => g,
    y => p25);


  u27 : g2 port map (
    a => p11,
    b => p20,
    y => t20);


  u28 : g2 port map (
    a => p12,
    b => p19,
    y => t21);


  u29 : g2 port map (
    a => p13,
    b => p17,
    y => t22);


  u30 : g2 port map (
    a => p9,
    b => p21,
    y => t23);


  u31 : g4 port map (
    a => t20,
    b => t21,
    c => t22,
    d => t23,
    y => t24,
    y2 => p22_p23);

  tp5 <= t24;

  u32 : g2 port map (
    a => t24,
    b => p16,
    y => p15);

  u32 : g2 port map (
    a => t24,
    b => p4,
    y => p18);


end gates;

