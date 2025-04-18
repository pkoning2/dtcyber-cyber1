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
-- CP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cp is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp5 : out logicsig;
      p1_tp3 : out logicsig;
      p2_tp1 : out logicsig;
      p15_tp2 : out logicsig;
      p27_tp6 : out logicsig;
      p28_tp4 : out logicsig);

end cp;
architecture gates of cp is
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

  signal a : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g5 port map (
    a => p3,
    b => p5,
    c => p7,
    d => p9,
    e => p11,
    y => f);


  u2 : g2 port map (
    a => f,
    b => a,
    y => p2_tp1);


  u3 : g2 port map (
    a => c,
    b => d,
    y => t2);

  tp5 <= c;

  u5 : g2 port map (
    a => c,
    b => e,
    y => t3);


  u6 : g2 port map (
    a => c,
    b => f,
    y => t4);


  u7 : g3 port map (
    a => t2,
    b => t3,
    c => t4,
    y => p15_tp2);


  u8 : g5 port map (
    a => p4,
    b => p6,
    c => p8,
    d => p10,
    e => p12,
    y => e);


  u9 : g2 port map (
    a => e,
    b => a,
    y => p1_tp3);


  u10 : g5 port map (
    a => p17,
    b => p19,
    c => p21,
    d => p23,
    e => p25,
    y2 => p28_tp4);


  u11 : inv port map (
    a => p16,
    y => c);


  u12 : g2 port map (
    a => p14,
    b => p13,
    y2 => a);


  u13 : g5 port map (
    a => p18,
    b => p20,
    c => p22,
    d => p24,
    e => p26,
    y => d);


  u14 : g2 port map (
    a => d,
    b => a,
    y => p27_tp6);



end gates;

