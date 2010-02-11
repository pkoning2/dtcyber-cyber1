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
-- HT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ht is
    port (
      p1 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p11 : out logicsig;
      p16 : out logicsig;
      p17_tp6 : out logicsig;
      p20 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end ht;
architecture gates of ht is
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

begin -- gates
  u1 : g2 port map (
    a => p27,
    b => a,
    y2 => p28);

  tp4 <= a;

  u2 : g2 port map (
    a => p23,
    b => a,
    y2 => p24);


  u3 : g2 port map (
    a => p19,
    b => a,
    y2 => p20);


  u4 : inv port map (
    a => p14,
    y => a);


  u5 : inv port map (
    a => p15,
    y => b);

  tp5 <= b;

  u6 : inv port map (
    a => p12,
    y => c);

  tp2 <= c;

  u7 : inv port map (
    a => p13,
    y => d);

  tp3 <= d;

  u8 : inv port map (
    a => p9,
    y => e);

  tp1 <= e;

  u9 : g2 port map (
    a => p26,
    b => b,
    y2 => p25);


  u10 : g2 port map (
    a => p10,
    b => b,
    y2 => p16);


  u11 : g2 port map (
    a => p1,
    b => c,
    y2 => p2);


  u12 : g2 port map (
    a => p1,
    b => d,
    y2 => p3);


  u13 : g2 port map (
    a => p6,
    b => c,
    y2 => p4);


  u14 : g2 port map (
    a => p6,
    b => d,
    y2 => p5);


  u15 : g2 port map (
    a => p22,
    b => e,
    y2 => p11);


  u16 : g2 port map (
    a => p8,
    b => e,
    y2 => p7);


  u17 : g2 port map (
    a => p21,
    b => p18,
    y => p17_tp6);



end gates;

