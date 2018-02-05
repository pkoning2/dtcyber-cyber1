-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2018 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- UM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity umslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      i6 : in  logicsig;
      q : out logicsig);

end umslice;
architecture gates of umslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => a,
    y => t1);


  u2 : g2 port map (
    a => i2,
    b => b,
    y => t2);


  u3 : g2 port map (
    a => i3,
    b => c,
    y => t3);


  u4 : g2 port map (
    a => i4,
    b => d,
    y => t4);


  u5 : g2 port map (
    a => i5,
    b => e,
    y => t5);


  u6 : g2 port map (
    a => i6,
    b => f,
    y => t6);


  u7 : g6 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => t5,
    f => t6,
    y => q);



end gates;

use work.sigs.all;

entity um is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
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
      p28 : in  logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig);

end um;
architecture gates of um is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component umslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      i6 : in  logicsig;
      q : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;

begin -- gates
  u1 : inv port map (
    a => p10,
    y => a);


  u2 : inv port map (
    a => p20,
    y => b);


  u3 : inv port map (
    a => p21,
    y => c);


  u4 : inv port map (
    a => p28,
    y => d);


  u5 : inv port map (
    a => p8,
    y => e);


  u6 : inv port map (
    a => p1,
    y => f);


  u7 : umslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p1,
    i2 => p19,
    i3 => p22,
    i4 => p23,
    i5 => p7,
    i6 => p6,
    q => p16);


  u8 : umslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p12,
    i2 => p18,
    i3 => p26,
    i4 => p27,
    i5 => p3,
    i6 => p2,
    q => p14);


  u9 : umslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p11,
    i2 => p17,
    i3 => p24,
    i4 => p25,
    i5 => p5,
    i6 => p4,
    q => p13);



end gates;

