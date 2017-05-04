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
-- RC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rc is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p16 : in  logicsig;
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
      p5 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig);

end rc;
architecture gates of rc is
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
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p6,
    y => d);

  tp2 <= d;

  u2 : inv port map (
    a => p3,
    y => c);

  tp3 <= c;

  u3 : inv port map (
    a => p16,
    y => a);

  tp4 <= a;

  u4 : inv port map (
    a => p25,
    y => b);

  tp6 <= b;

  u5 : g2 port map (
    a => p2,
    b => p4,
    y => t1,
    y2 => p13);

  tp1 <= t1;

  u6 : g2 port map (
    a => a,
    b => t1,
    y => p11);


  u7 : g2 port map (
    a => b,
    b => t1,
    y => p9);


  u8 : g2 port map (
    a => c,
    b => t1,
    y => p5);


  u9 : g2 port map (
    a => d,
    b => t1,
    y => p7);


  u10 : inv port map (
    a => p1,
    y => t2);


  u11 : g2 port map (
    a => t2,
    b => a,
    y => p8);


  u12 : g2 port map (
    a => t2,
    b => b,
    y => p10);


  u13 : g2 port map (
    a => t2,
    b => c,
    y => p14);


  u14 : g2 port map (
    a => t2,
    b => d,
    y => p12);


  u15 : g2 port map (
    a => p26,
    b => p28,
    y => t3,
    y2 => p15);

  tp5 <= t3;

  u16 : g2 port map (
    a => t3,
    b => a,
    y => p23);


  u17 : g2 port map (
    a => t3,
    b => b,
    y => p21);


  u18 : g2 port map (
    a => t3,
    b => c,
    y => p19);


  u19 : g2 port map (
    a => t3,
    b => d,
    y => p17);


  u20 : inv port map (
    a => p27,
    y => t4);


  u21 : g2 port map (
    a => t4,
    b => a,
    y => p18);


  u22 : g2 port map (
    a => t4,
    b => b,
    y => p20);


  u23 : g2 port map (
    a => t4,
    b => c,
    y => p22);


  u24 : g2 port map (
    a => t4,
    b => d,
    y => p24);



end gates;

