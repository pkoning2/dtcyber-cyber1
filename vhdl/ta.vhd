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
-- TA module, rev C -- 6 input gates
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ta is
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
      p15 : in  logicsig;
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
      p1_tp2 : out logicsig;
      p2_tp1 : out logicsig;
      p27_tp6 : out logicsig;
      p28_tp5 : out logicsig);

end ta;
architecture gates of ta is
  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : g6 port map (
    a => p14,
    b => p12,
    c => p10,
    d => p8,
    e => p6,
    f => p4,
    y => p2_tp1);


  u2 : g6 port map (
    a => p13,
    b => p11,
    c => p9,
    d => p7,
    e => p5,
    f => p3,
    y => p1_tp2);


  u3 : g6 port map (
    a => p16,
    b => p18,
    c => p20,
    d => p22,
    e => p24,
    f => p26,
    y => p28_tp5);


  u4 : g6 port map (
    a => p15,
    b => p17,
    c => p19,
    d => p21,
    e => p23,
    f => p25,
    y => p27_tp6);



end gates;

