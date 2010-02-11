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
-- IO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity io is
    port (
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p11_tp3 : out logicsig;
      p13 : out logicsig;
      p14_p15_p17 : out logicsig;
      p18 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end io;
architecture gates of io is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p8,
    y2 => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => c,
    b => t1,
    y => p4);


  u3 : g2 port map (
    a => b,
    b => t1,
    y => p2);


  u4 : g2 port map (
    a => a,
    b => t1,
    y => p6);


  u5 : inv2 port map (
    a => p7,
    y2 => t2);

  tp2 <= t2;

  u6 : g2 port map (
    a => a,
    b => t2,
    y => p5);


  u7 : g2 port map (
    a => b,
    b => t2,
    y => p1);


  u8 : g2 port map (
    a => c,
    b => t2,
    y => p3);


  u9 : inv port map (
    a => p9,
    y => a);


  u10 : inv2 port map (
    a => a,
    y2 => p11_tp3);


  u11 : inv port map (
    a => p19,
    y => b);


  u12 : inv2 port map (
    a => b,
    y2 => p13);


  u13 : inv port map (
    a => p20,
    y => c);


  u14 : inv2 port map (
    a => c,
    y2 => p18);


  u15 : inv2 port map (
    a => p21,
    y2 => t4);

  tp5 <= t4;

  u16 : g2 port map (
    a => a,
    b => t4,
    y => p25);


  u17 : g2 port map (
    a => b,
    b => t4,
    y => p27);


  u18 : g2 port map (
    a => c,
    b => t4,
    y => p23);


  u19 : inv2 port map (
    a => p22,
    y2 => t5);

  tp6 <= t5;

  u20 : g2 port map (
    a => a,
    b => t5,
    y => p26);


  u21 : g2 port map (
    a => b,
    b => t5,
    y => p28);


  u22 : g2 port map (
    a => c,
    b => t5,
    y => p24);


  u23 : g2 port map (
    a => p12,
    b => p10,
    y => t6);


  u24 : rsflop port map (
    r => p16,
    s => t6,
    q => t7);

  tp4 <= t7;

  u25 : inv port map (
    a => t7,
    y => p14_p15_p17);



end gates;

