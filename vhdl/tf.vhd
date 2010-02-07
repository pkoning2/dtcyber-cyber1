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
-- TF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      y : out logicsig);

end tfslice;
architecture gates of tfslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

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


  u5 : g5 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => e,
    y => y);



end gates;

use work.sigs.all;

entity tf is
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
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p19 : out logicsig);

end tf;
architecture gates of tf is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component tfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p1,
    y => a);


  u2 : inv port map (
    a => p27,
    y => b);


  u3 : inv port map (
    a => p2,
    y => c);


  u4 : inv port map (
    a => p28,
    y => d);


  u5 : tfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p9,
    i1 => p8,
    i2 => p22,
    i3 => p7,
    i4 => p21,
    y => e);

  tp5 <= e;

  u6 : tfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p20,
    i1 => p6,
    i2 => p24,
    i3 => p5,
    i4 => p23,
    y => f);

  tp6 <= f;

  u7 : tfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p10,
    i1 => p4,
    i2 => p26,
    i3 => p3,
    i4 => p25,
    y => g);


  u8 : g3 port map (
    a => e,
    b => f,
    c => g,
    y => t1);

  p11 <= t1;
  p13 <= t1;
  p19 <= t1;
  tp1 <= t1;


end gates;

