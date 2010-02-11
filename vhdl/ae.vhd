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
-- AE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity aeslice is
    port (
      a : in  logicsig;
      r1 : in  logicsig;
      r2 : in  logicsig;
      r3 : in  logicsig;
      s1 : in  logicsig;
      s2 : in  logicsig;
      q_tp : out logicsig;
      qb : out logicsig);

end aeslice;
architecture gates of aeslice is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => a,
    b => s1,
    c => s2,
    y => t1);


  u2 : g3 port map (
    a => r1,
    b => r2,
    c => a,
    y => t2);


  u3 : r2sflop port map (
    r => t2,
    r2 => r3,
    s => t1,
    q => q_tp,
    qb => qb);



end gates;

use work.sigs.all;

entity ae is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p8_tp1 : out logicsig;
      p9 : out logicsig;
      p10_tp2 : out logicsig;
      p18 : out logicsig;
      p20_tp5 : out logicsig;
      p25_tp6 : out logicsig;
      p27 : out logicsig);

end ae;
architecture gates of ae is
  component aeslice
    port (
      a : in  logicsig;
      r1 : in  logicsig;
      r2 : in  logicsig;
      r3 : in  logicsig;
      s1 : in  logicsig;
      s2 : in  logicsig;
      q_tp : out logicsig;
      qb : out logicsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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
  signal c : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p16,
    y2 => a);


  u2 : g2 port map (
    a => p19,
    b => p17,
    y2 => c);


  u3 : aeslice port map (
    a => a,
    r1 => p2,
    r2 => p4,
    r3 => c,
    s1 => p3,
    s2 => p5,
    q_tp => p8_tp1,
    qb => p1);


  u4 : aeslice port map (
    a => a,
    r1 => p14,
    r2 => p12,
    r3 => c,
    s1 => p11,
    s2 => p7,
    q_tp => p10_tp2,
    qb => p9);


  u5 : aeslice port map (
    a => a,
    r1 => p24,
    r2 => p22,
    r3 => c,
    s1 => p15,
    s2 => p13,
    q_tp => p20_tp5,
    qb => p18);


  u6 : aeslice port map (
    a => a,
    r1 => p28,
    r2 => p26,
    r3 => c,
    s1 => p21,
    s2 => p23,
    q_tp => p25_tp6,
    qb => p27);



end gates;

