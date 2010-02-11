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
-- JY module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jyslice is
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      s : in  logicsig;
      q1_tp : out logicsig;
      q2_q3 : out logicsig;
      q4 : out logicsig);

end jyslice;
architecture gates of jyslice is
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : rs2flop port map (
    r => s,
    s => i1,
    s2 => i2,
    q => t1);

  q1_tp <= t1;

  u2 : inv2 port map (
    a => t1,
    y2 => t2);

  q2_q3 <= t2;

  u3 : inv port map (
    a => t2,
    y => q4);



end gates;

use work.sigs.all;

entity jy is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p2_p4 : out logicsig;
      p6_tp3 : out logicsig;
      p7 : out logicsig;
      p12 : out logicsig;
      p13_tp2 : out logicsig;
      p16 : out logicsig;
      p17_tp4 : out logicsig;
      p20_p21 : out logicsig;
      p22 : out logicsig;
      p25_tp6 : out logicsig;
      p26_p28 : out logicsig;
      p27 : out logicsig);

end jy;
architecture gates of jy is
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

  component jyslice
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      s : in  logicsig;
      q1_tp : out logicsig;
      q2_q3 : out logicsig;
      q4 : out logicsig);

  end component;

  signal s : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p9,
    y => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => p8,
    b => t1,
    y => p7);


  u3 : g2 port map (
    a => p10,
    b => t1,
    y => p12);


  u4 : g2 port map (
    a => t1,
    b => p14,
    y => p16);


  u5 : inv port map (
    a => p11,
    y => p13_tp2);


  u6 : inv port map (
    a => p15,
    y => s);

  tp5 <= s;

  u7 : jyslice port map (
    i1 => p5,
    i2 => p3,
    s => s,
    q1_tp => p6_tp3,
    q2_q3 => p2_p4,
    q4 => p1);


  u8 : jyslice port map (
    i1 => p18,
    i2 => p19,
    s => s,
    q1_tp => p17_tp4,
    q2_q3 => p20_p21,
    q4 => p22);


  u9 : jyslice port map (
    i1 => p23,
    i2 => p24,
    s => s,
    q1_tp => p25_tp6,
    q2_q3 => p26_p28,
    q4 => p27);



end gates;

