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
-- NM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nmslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig);

end nmslice;
architecture gates of nmslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => a,
    y => t1);


  u2 : g2 port map (
    a => i2,
    b => a,
    y => t2);


  u3 : rsflop port map (
    r => b,
    s => t1,
    q => t3,
    qb => t4);

  y1 <= t4;

  u4 : g2 port map (
    a => f,
    b => t4,
    y => t5);


  u5 : rsflop port map (
    r => b,
    s => t2,
    q => y2,
    qb => t6);


  u6 : g2 port map (
    a => t6,
    b => e,
    y => t7);


  u7 : g2 port map (
    a => t6,
    b => t3,
    y => y4);


  u8 : g2 port map (
    a => t5,
    b => t7,
    y => y3);



end gates;

use work.sigs.all;

entity nm is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p13 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p3_tp2 : out logicsig;
      p5 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12_tp1 : out logicsig;
      p15_tp3 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18_tp4 : out logicsig;
      p19_tp6 : out logicsig;
      p21 : out logicsig;
      p26 : out logicsig;
      p28_tp5 : out logicsig);

end nm;
architecture gates of nm is
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

  component nmslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal e : logicsig;
  signal f : logicsig;

begin -- gates
  u1 : inv port map (
    a => p8,
    y => a);


  u2 : inv port map (
    a => p23,
    y => b);


  u3 : inv2 port map (
    a => p13,
    y => e,
    y2 => f);

  p16 <= f;

  u4 : nmslice port map (
    a => a,
    b => b,
    e => e,
    f => f,
    i1 => p4,
    i2 => p6,
    y1 => p10,
    y2 => p5,
    y3 => p12_tp1,
    y4 => p3_tp2);


  u5 : nmslice port map (
    a => a,
    b => b,
    e => e,
    f => f,
    i1 => p7,
    i2 => p24,
    y1 => p11,
    y2 => p17,
    y3 => p15_tp3,
    y4 => p18_tp4);


  u6 : nmslice port map (
    a => a,
    b => b,
    e => e,
    f => f,
    i1 => p27,
    i2 => p25,
    y1 => p21,
    y2 => p26,
    y3 => p19_tp6,
    y4 => p28_tp5);



end gates;

