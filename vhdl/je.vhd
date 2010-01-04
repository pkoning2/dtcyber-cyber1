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
-- JE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jeslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      y : out logicsig);

end jeslice;
architecture gates of jeslice is
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

  component rs4flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => i1,
    y2 => y);


  u2 : g2 port map (
    a => i1,
    b => a,
    y => t1);


  u3 : rs4flop port map (
    r => d,
    s => i2,
    s2 => i3,
    s3 => t1,
    s4 => '1',
    q => t2);


  u4 : g2 port map (
    a => t2,
    b => b,
    y => t3);

  tp <= t2;

  u5 : g2 port map (
    a => i4,
    b => t3,
    y => q1);


  u6 : g2 port map (
    a => t2,
    b => c,
    y => q2);



end gates;

use work.sigs.all;

entity je is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p12 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end je;
architecture gates of je is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component jeslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv port map (
    a => p18,
    y => a);


  u2 : inv port map (
    a => p15,
    y => b);

  tp2 <= b;

  u3 : inv port map (
    a => p14,
    y => c);


  u4 : inv port map (
    a => p16,
    y => d);


  u5 : jeslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p1,
    i2 => p8,
    i3 => p10,
    i4 => p2,
    q1 => p4,
    q2 => p3,
    tp => tp1,
    y => p6);


  u6 : jeslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p20,
    i2 => p7,
    i3 => p9,
    i4 => p13,
    q1 => p19,
    q2 => p12,
    tp => tp5,
    y => p21);


  u7 : jeslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p25,
    i2 => p22,
    i3 => p24,
    i4 => p27,
    q1 => p28,
    q2 => p23,
    tp => tp6,
    y => p26);



end gates;

