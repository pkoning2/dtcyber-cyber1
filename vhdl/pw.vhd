-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- PW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pw is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p8 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pw;
architecture gates of pw is
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

  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal j : logicsig;
  signal k : logicsig;
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
  u1 : inv2 port map (
    a => p2,
    y => f,
    y2 => g);


  u2 : inv2 port map (
    a => p4,
    y => j,
    y2 => k);


  u3 : inv port map (
    a => p7,
    y => e);


  u4 : g2 port map (
    a => p5,
    b => f,
    y => t1);


  u5 : g2 port map (
    a => g,
    b => p11,
    y => t2);


  u6 : g2 port map (
    a => p9,
    b => j,
    y => t3);


  u7 : g2 port map (
    a => k,
    b => p3,
    y => t4);


  u8 : g4 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    y => t5);

  tp1 <= t5;

  u9 : g2 port map (
    a => t5,
    b => e,
    y => p8);


  u10 : g2 port map (
    a => p22,
    b => e,
    y => p23,
    y2 => t6);


  u11 : inv port map (
    a => p17,
    y => d);


  u13 : g3 port map (
    a => p19,
    b => t6,
    c => d,
    y => t7,
    y2 => t8);

  p21 <= t7;
  tp5 <= t7;

  u14 : g2 port map (
    a => p16,
    b => t8,
    y => p14);


  u15 : g2 port map (
    a => t8,
    b => p10,
    y => p12);


  u16 : g2 port map (
    a => p26,
    b => e,
    y => p24,
    y2 => t9);


  u17 : g3 port map (
    a => d,
    b => p20,
    c => t9,
    y => t10);

  p27 <= t10;
  tp6 <= t10;

  u18 : g2 port map (
    a => t7,
    b => t10,
    y => p25,
    y2 => p28);



end gates;

