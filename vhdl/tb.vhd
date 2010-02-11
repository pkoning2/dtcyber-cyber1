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
-- TB module, rev C -- 3-6 input gates
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tb is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p12_p13_p15_p17 : out logicsig;
      p14_p16_p18 : out logicsig;
      p19 : out logicsig;
      p20_p22 : out logicsig);

end tb;
architecture gates of tb is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

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
    a => p1,
    b => p3,
    c => p5,
    d => p7,
    e => p9,
    f => p11,
    y => tp1,
    y2 => p19);


  u2 : g5 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p8,
    e => p10,
    y => tp2,
    y2 => p12_p13_p15_p17);


  u3 : g4 port map (
    a => p21,
    b => p23,
    c => p25,
    d => p27,
    y => tp5,
    y2 => p14_p16_p18);


  u4 : g3 port map (
    a => p24,
    b => p26,
    c => p28,
    y => tp6,
    y2 => p20_p22);



end gates;

