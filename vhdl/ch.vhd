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
-- CH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ch is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7_tp2 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21_tp6 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end ch;
architecture gates of ch is
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

  signal a : logicsig;
  signal c : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p22,
    y => a);

  tp5 <= a;

  u2 : inv port map (
    a => p10,
    y => c);

  tp1 <= c;

  u4 : inv port map (
    a => p2,
    y => t1);


  u5 : g2 port map (
    a => a,
    b => t1,
    y => p1);


  u6 : g2 port map (
    a => c,
    b => t1,
    y => p3);


  u7 : inv port map (
    a => p4,
    y => t2);


  u8 : g2 port map (
    a => a,
    b => t2,
    y => p5);


  u9 : g2 port map (
    a => c,
    b => t2,
    y => p7_tp2);


  u10 : inv port map (
    a => p15,
    y => t4);


  u11 : g2 port map (
    a => a,
    b => t4,
    y => p9);


  u12 : g2 port map (
    a => c,
    b => t4,
    y => p11);


  u13 : inv port map (
    a => p12,
    y => t5);


  u14 : g2 port map (
    a => a,
    b => t5,
    y => p13);


  u15 : g2 port map (
    a => c,
    b => t5,
    y => p16);


  u16 : inv port map (
    a => p14,
    y => t6);


  u17 : g2 port map (
    a => a,
    b => t6,
    y => p18);


  u18 : g2 port map (
    a => c,
    b => t6,
    y => p20);


  u19 : inv port map (
    a => p24,
    y => t7);


  u20 : g2 port map (
    a => a,
    b => t7,
    y => p17);


  u21 : g2 port map (
    a => c,
    b => t7,
    y => p19);


  u22 : inv port map (
    a => p26,
    y => t8);


  u23 : g2 port map (
    a => a,
    b => t8,
    y => p27);


  u24 : g2 port map (
    a => c,
    b => t8,
    y => p25);


  u25 : inv port map (
    a => p28,
    y => t9);


  u26 : g2 port map (
    a => a,
    b => t9,
    y => p23);


  u27 : g2 port map (
    a => c,
    b => t9,
    y => p21_tp6);



end gates;

