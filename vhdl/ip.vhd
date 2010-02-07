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
-- IP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ip is
    port (
      p1 : in  logicsig;
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
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p28 : out logicsig);

end ip;
architecture gates of ip is
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
    a => p4,
    b => p6,
    c => p8,
    d => p10,
    e => p12,
    f => p14,
    y => tp1,
    y2 => p13);


  u2 : g6 port map (
    a => p1,
    b => p3,
    c => p5,
    d => p7,
    e => p9,
    f => p11,
    y => tp2,
    y2 => p2);


  u3 : g6 port map (
    a => p17,
    b => p19,
    c => p21,
    d => p23,
    e => p25,
    f => p27,
    y => tp5,
    y2 => p28);


  u4 : g6 port map (
    a => p16,
    b => p18,
    c => p20,
    d => p22,
    e => p24,
    f => p26,
    y => tp6,
    y2 => p15);



end gates;

