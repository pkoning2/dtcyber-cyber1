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
-- PP module - rev B
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pp is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p10 : out logicsig;
      p13_tp2 : out logicsig;
      p14 : out logicsig;
      p17_tp5 : out logicsig;
      p18 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end pp;
architecture gates of pp is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p1,
    b => p3,
    y => t1);


  u2 : g2 port map (
    a => p12,
    b => p9,
    y => t2);


  u3 : inv port map (
    a => p7,
    y => t3);


  u4 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => a,
    y2 => t4);

  tp1 <= a;

  u5 : inv port map (
    a => p16,
    y => t5);


  u6 : g4 port map (
    a => p19,
    b => t5,
    c => p21,
    d => t4,
    y => tp6,
    y2 => p14);


  u7 : g2 port map (
    a => p2,
    b => a,
    y => p4);


  u8 : g2 port map (
    a => p6,
    b => a,
    y => p5);


  u9 : g2 port map (
    a => p8,
    b => a,
    y => p10);


  u10 : g2 port map (
    a => a,
    b => p11,
    y => p13_tp2);


  u11 : g2 port map (
    a => p20,
    b => a,
    y => p18);


  u12 : g2 port map (
    a => p23,
    b => a,
    y => p22);


  u13 : g2 port map (
    a => p27,
    b => a,
    y => p25);


  u14 : g2 port map (
    a => p28,
    b => a,
    y => p26);


  u15 : g2 port map (
    a => a,
    b => p15,
    y => p17_tp5);



end gates;

