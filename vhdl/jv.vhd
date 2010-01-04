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
-- JV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jv is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
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
      p22 : out logicsig;
      p23 : out logicsig;
      p28 : out logicsig);

end jv;
architecture gates of jv is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p6,
    y => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => t1,
    b => a,
    y => p2);


  u3 : g2 port map (
    a => t1,
    b => b,
    y => p14);


  u4 : g2 port map (
    a => t1,
    b => c,
    y => p16);


  u5 : g2 port map (
    a => t1,
    b => d,
    y => p4);


  u6 : g2 port map (
    a => t1,
    b => e,
    y => p12);


  u7 : g2 port map (
    a => t1,
    b => f,
    y => p7);


  u8 : inv port map (
    a => p3,
    y => t2);

  tp2 <= t2;

  u9 : g2 port map (
    a => t2,
    b => a,
    y => p10);


  u10 : g2 port map (
    a => t2,
    b => b,
    y => p11);


  u11 : g2 port map (
    a => t2,
    b => c,
    y => p13);


  u12 : g2 port map (
    a => t2,
    b => d,
    y => p23);


  u13 : g2 port map (
    a => t2,
    b => e,
    y => p18);


  u14 : inv2 port map (
    a => p24,
    y => tp5,
    y2 => t3);

  p9 <= t3;
  p15 <= t3;
  p17 <= t3;
  p19 <= t3;

  u15 : inv2 port map (
    a => p21,
    y => tp6,
    y2 => t4);

  p22 <= t4;
  p28 <= t4;

  u16 : inv port map (
    a => p8,
    y => a);


  u17 : inv port map (
    a => p5,
    y => b);


  u18 : inv port map (
    a => p26,
    y => c);


  u19 : inv port map (
    a => p25,
    y => d);


  u20 : inv port map (
    a => p27,
    y => e);


  u21 : inv port map (
    a => p20,
    y => f);



end gates;

