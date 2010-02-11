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
-- AM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity am is
    port (
      p10 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p1_p2_p3_p4_p5_p6_p7_p9_p11_p13 : out logicsig;
      p8_p14_p19_p20_p21_p22_p23_p24_p25_p26_p28 : out logicsig);

end am;
architecture gates of am is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

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
  signal t : logicsig;

begin -- gates
  u1 : g4 port map (
    a => b,
    b => c,
    c => d,
    d => t,
    y => tp1,
    y2 => p1_p2_p3_p4_p5_p6_p7_p9_p11_p13);

  tp2 <= d;
  tp3 <= c;
  tp4 <= b;

  u2 : g4 port map (
    a => a,
    b => c,
    c => d,
    d => t,
    y => tp6,
    y2 => p8_p14_p19_p20_p21_p22_p23_p24_p25_p26_p28);


  u3 : inv port map (
    a => p15,
    y => t);


  u4 : inv port map (
    a => p27,
    y => a);


  u5 : inv port map (
    a => p10,
    y => e);


  u6 : g2 port map (
    a => p27,
    b => e,
    y => b);


  u7 : g3 port map (
    a => e,
    b => p18,
    c => p17,
    y => c);


  u8 : g3 port map (
    a => p12,
    b => e,
    c => p16,
    y => d);



end gates;

