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
-- RF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

end rfslice;
architecture gates of rfslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => c,
    y => t1);


  u2 : inv port map (
    a => c,
    y => t2);

  y <= t2;

  u3 : g2 port map (
    a => b,
    b => t2,
    y => t3);


  u4 : g2 port map (
    a => t1,
    b => t3,
    y => y2);



end gates;

use work.sigs.all;

entity rf is
    port (
      p3 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p11 : out logicsig;
      p12_tp2 : out logicsig;
      p14 : out logicsig;
      p18 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig);

end rf;
architecture gates of rf is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p9,
    b => p17,
    y => t1);


  u2 : g2 port map (
    a => p19,
    b => p21,
    y => t2);


  u3 : g2 port map (
    a => p22,
    b => p23,
    y => t3);


  u4 : g2 port map (
    a => p15,
    b => p13,
    y => t4);


  u5 : g4 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    y => p12_tp2);


  u6 : inv2 port map (
    a => p20,
    y2 => a);

  tp5 <= a;

  u7 : inv2 port map (
    a => p8,
    y2 => b);

  tp1 <= b;

  u8 : rfslice port map (
    a => p20,
    b => p8,
    c => p7,
    y => p5,
    y2 => p11);


  u9 : rfslice port map (
    a => a,
    b => b,
    c => p16,
    y => p18,
    y2 => p14);


  u10 : rfslice port map (
    a => a,
    b => b,
    c => p6,
    y => p2,
    y2 => p4);


  u11 : rfslice port map (
    a => a,
    b => b,
    c => p3,
    y2 => p1);


  u12 : rfslice port map (
    a => a,
    b => b,
    c => p24,
    y => tp6,
    y2 => p25);


  u13 : rfslice port map (
    a => a,
    b => b,
    c => p28,
    y2 => p26);


  p27 <= '0';

end gates;

