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
-- IV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity iv is
    port (
      p6 : in  logicsig;
      p8 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p28 : out logicsig);

end iv;
architecture gates of iv is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

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
  u1 : inv2 port map (
    a => p6,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p8,
    y => c,
    y2 => d);


  u3 : inv2 port map (
    a => p26,
    y => e,
    y2 => f);


  u4 : g3 port map (
    a => a,
    b => c,
    c => e,
    y => t1,
    y2 => p16);

  p23 <= t1;
  tp6 <= t1;

  u5 : g3 port map (
    a => b,
    b => c,
    c => e,
    y => p1,
    y2 => p10);


  u6 : g3 port map (
    a => a,
    b => d,
    c => e,
    y => t2,
    y2 => p14);

  p21 <= t2;
  tp1 <= t2;

  u7 : g3 port map (
    a => b,
    b => d,
    c => e,
    y => t3,
    y2 => p11);

  p20 <= t3;
  tp5 <= t3;

  u8 : g3 port map (
    a => a,
    b => c,
    c => f,
    y => t4,
    y2 => p12);

  p17 <= t4;
  tp2 <= t4;

  u9 : g3 port map (
    a => b,
    b => c,
    c => f,
    y => p4,
    y2 => p9);


  u10 : g3 port map (
    a => a,
    b => d,
    c => f,
    y => p2,
    y2 => p13);


  u11 : g3 port map (
    a => b,
    b => d,
    c => f,
    y => p28,
    y2 => p15);



end gates;

