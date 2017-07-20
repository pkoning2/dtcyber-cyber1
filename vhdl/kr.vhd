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
-- KR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity krslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      m : in  logicsig;
      g : out logicsig;
      j : out logicsig;
      q : out logicsig;
      qb : out logicsig);

end krslice;
architecture gates of krslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => a,
    y => t1,
    y2 => t7);


  u2 : inv2 port map (
    a => b,
    y => t2,
    y2 => t3);


  u3 : g2 port map (
    a => t7,
    b => t3,
    y => g);


  u4 : g2 port map (
    a => t1,
    b => t3,
    y => t4);


  u5 : g2 port map (
    a => t2,
    b => t7,
    y => t5);


  u6 : g2 port map (
    a => t4,
    b => t5,
    y => t6);


  u7 : g2 port map (
    a => m,
    b => t6,
    y => q,
    y2 => qb);



end gates;

use work.sigs.all;

entity kr is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p1_p2 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p13_tp3 : out logicsig;
      p14_tp4 : out logicsig;
      p16_tp1 : out logicsig;
      p18_tp5 : out logicsig;
      p26 : out logicsig;
      p27_tp7 : out logicsig;
      p28 : out logicsig);

end kr;
architecture gates of kr is
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

  component krslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      m : in  logicsig;
      g : out logicsig;
      j : out logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p11,
    y => m);


  u2 : krslice port map (
    a => p22,
    b => p24,
    m => m,
    g => g,
    j => j,
    q => p26,
    qb => p28);


  u3 : krslice port map (
    a => p7,
    b => p5,
    m => m,
    g => t1,
    j => i,
    q => p3,
    qb => l);

  p1_p2 <= l;
  p6 <= i;

  u4 : g2 port map (
    a => t1,
    b => m,
    y => e);

  p16_tp1 <= e;

  u5 : krslice port map (
    a => p20,
    b => p9,
    m => m,
    g => t2,
    j => h,
    q => p8,
    qb => p27_tp7);

  p10 <= h;

  u6 : g2 port map (
    a => t2,
    b => m,
    y => d);

  p13_tp3 <= d;

  u7 : g3 port map (
    a => h,
    b => l,
    c => j,
    y => p14_tp4);


  u8 : g3 port map (
    a => d,
    b => i,
    c => j,
    y => t3);


  u9 : g2 port map (
    a => e,
    b => j,
    y => t4);


  u10 : g3 port map (
    a => t3,
    b => g,
    c => t4,
    y => t5);


  u11 : g2 port map (
    a => t5,
    b => m,
    y => p18_tp5);



end gates;

