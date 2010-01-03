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
-- IR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ir is
    port (
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ir;
architecture gates of ir is
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
    a => p16,
    y => a);


  u2 : inv port map (
    a => p14,
    y => b);


  u3 : inv port map (
    a => p15,
    y => c);


  u4 : inv port map (
    a => p13,
    y => d);


  u5 : g2 port map (
    a => p11,
    b => p9,
    y => t1);

  tp1 <= t1;

  u6 : g2 port map (
    a => t1,
    b => a,
    y => p8);


  u7 : g2 port map (
    a => t1,
    b => b,
    y => p6);


  u8 : g2 port map (
    a => t1,
    b => c,
    y => p4);


  u9 : g2 port map (
    a => t1,
    b => d,
    y => p2);


  u10 : g2 port map (
    a => p12,
    b => p10,
    y => t2);

  tp2 <= t2;

  u11 : g2 port map (
    a => t2,
    b => a,
    y => p7);


  u12 : g2 port map (
    a => t2,
    b => b,
    y => p5);


  u13 : g2 port map (
    a => t2,
    b => c,
    y => p3);


  u14 : g2 port map (
    a => t2,
    b => d,
    y => p1);


  u15 : g2 port map (
    a => p19,
    b => p17,
    y => t3);

  tp3 <= t3;

  u16 : g2 port map (
    a => t3,
    b => a,
    y => p22);


  u17 : g2 port map (
    a => t3,
    b => b,
    y => p24);


  u18 : g2 port map (
    a => t3,
    b => c,
    y => p26);


  u19 : g2 port map (
    a => t3,
    b => d,
    y => p28);


  u20 : g2 port map (
    a => p18,
    b => p20,
    y => t4);

  tp4 <= t4;

  u21 : g2 port map (
    a => t4,
    b => a,
    y => p21);


  u22 : g2 port map (
    a => t4,
    b => b,
    y => p23);


  u23 : g2 port map (
    a => t4,
    b => c,
    y => p25);


  u24 : g2 port map (
    a => t4,
    b => d,
    y => p27);



end gates;

