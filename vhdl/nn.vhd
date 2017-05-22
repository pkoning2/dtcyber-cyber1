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
-- NN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nnslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      x : out logicsig;
      y : out logicsig);

end nnslice;
architecture gates of nnslice is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => i2,
    y => t1);


  u2 : inv port map (
    a => i2,
    y => t2);


  u3 : g2 port map (
    a => t2,
    b => i3,
    y => t4);


  u4 : g2 port map (
    a => t1,
    b => t4,
    y => t5);


  u5 : g2 port map (
    a => t5,
    b => b,
    y => x);


  u6 : g3 port map (
    a => t1,
    b => t4,
    c => a,
    y => y);



end gates;

use work.sigs.all;

entity nn is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp3 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end nn;
architecture gates of nn is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component nnslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      x : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal x : logicsig;
  signal y : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p15,
    y => b,
    y2 => a);

  tp3 <= a;

  u2 : nnslice port map (
    a => a,
    b => b,
    i1 => p5,
    i2 => p10,
    i3 => p8,
    x => x,
    y => y);


  u3 : nnslice port map (
    a => a,
    b => b,
    i1 => p6,
    i2 => p4,
    i3 => p3,
    x => t1,
    y => t2);


  u4 : g2 port map (
    a => t1,
    b => t2,
    y => p2,
    y2 => p1);


  u5 : nnslice port map (
    a => a,
    b => b,
    i1 => p23,
    i2 => p25,
    i3 => p26,
    x => t3,
    y => t4);


  u6 : g5 port map (
    a => x,
    b => y,
    c => t3,
    d => t4,
    e => p16,
    y => p27,
    y2 => p28);


  u7 : nnslice port map (
    a => a,
    b => b,
    i1 => p24,
    i2 => p22,
    i3 => p21,
    x => t5,
    y => t6);


  u8 : g5 port map (
    a => p16,
    b => x,
    c => y,
    d => t5,
    e => t6,
    y => p19,
    y2 => p17);



end gates;

