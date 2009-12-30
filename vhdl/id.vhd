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
-- ID module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity id is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig);

end id;
architecture gates of id is
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
  signal e : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p6,
    y => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => a,
    b => t1,
    y => p2);


  u3 : g2 port map (
    a => b,
    b => t1,
    y => p14);


  u4 : g2 port map (
    a => c,
    b => t1,
    y => p16);


  u5 : g2 port map (
    a => d,
    b => t1,
    y => p4);

  tp5 <= d;

  u6 : g2 port map (
    a => e,
    b => t1,
    y => p12);


  u7 : inv port map (
    a => p3,
    y => t2);

  tp2 <= t2;

  u8 : g2 port map (
    a => a,
    b => t2,
    y => p10);


  u9 : g2 port map (
    a => b,
    b => t2,
    y => p11);


  u10 : g2 port map (
    a => c,
    b => t2,
    y => p13);


  u11 : g2 port map (
    a => d,
    b => t2,
    y => p23);


  u12 : g2 port map (
    a => e,
    b => t2,
    y => p18);


  u13 : inv port map (
    a => p28,
    y => t3);

  tp6 <= t3;

  u14 : g2 port map (
    a => a,
    b => t3,
    y => p17);


  u15 : g2 port map (
    a => b,
    b => t3,
    y => p15);


  u16 : g2 port map (
    a => c,
    b => t3,
    y => p20);


  u17 : g2 port map (
    a => d,
    b => t3,
    y => p21);


  u18 : g2 port map (
    a => e,
    b => t3,
    y => p19);


  u19 : inv port map (
    a => p8,
    y => a);


  u20 : inv port map (
    a => p5,
    y => b);


  u21 : inv port map (
    a => p26,
    y => c);


  u22 : inv port map (
    a => p25,
    y => d);


  u23 : inv port map (
    a => p27,
    y => e);



end gates;

