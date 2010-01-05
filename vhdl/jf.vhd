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
-- JF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig);

end jfslice;
architecture gates of jfslice is
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
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => b,
    y => t1);


  u2 : inv2 port map (
    a => i2,
    y2 => t2);

  tp1 <= t2;
  y1 <= t2;

  u3 : g2 port map (
    a => i2,
    b => a,
    y => t3);


  u4 : rs2flop port map (
    r => c,
    s => t1,
    s2 => t3,
    q => t4);

  tp2 <= t4;

  u5 : g2 port map (
    a => i3,
    b => t4,
    y => y2);


  u6 : inv2 port map (
    a => t4,
    y => y3,
    y2 => y4);



end gates;

use work.sigs.all;

entity jf is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p21 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end jf;
architecture gates of jf is
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

  component jfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p18,
    y2 => a);


  u2 : inv2 port map (
    a => p16,
    y2 => b);


  u3 : inv port map (
    a => p15,
    y => c);


  u4 : jfslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p14,
    i2 => p8,
    i3 => p13,
    tp1 => tp1,
    tp2 => tp2,
    y1 => p11,
    y2 => p12,
    y3 => p10,
    y4 => p9);


  u5 : jfslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p4,
    i2 => p2,
    i3 => p5,
    tp2 => tp5,
    y1 => p1,
    y2 => p7,
    y3 => p6,
    y4 => p3);


  u6 : jfslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p23,
    i2 => p27,
    i3 => p24,
    tp2 => tp6,
    y1 => p28,
    y2 => p21,
    y3 => p25,
    y4 => p26);



end gates;

