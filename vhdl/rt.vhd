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
-- RT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rtslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

end rtslice;
architecture gates of rtslice is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t1);


  u2 : rsflop port map (
    r => d,
    s => t1,
    q => t2);

  y <= t2;

  u3 : inv port map (
    a => t2,
    y => y2);



end gates;

use work.sigs.all;

entity rtslice2 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end rtslice2;
architecture gates of rtslice2 is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => c,
    y => t1);


  u2 : rsflop port map (
    r => d,
    s => t1,
    q => t2);

  y <= t2;

  u3 : inv port map (
    a => t2,
    y => y2);


  u4 : g2 port map (
    a => t2,
    b => b,
    y => y3);



end gates;

use work.sigs.all;

entity rt is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig);

end rt;
architecture gates of rt is
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

  component rtslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rtslice2
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t : logicsig;
  signal u : logicsig;
  signal v : logicsig;
  signal x : logicsig;
  signal y : logicsig;
  signal z : logicsig;

begin -- gates
  u1 : inv port map (
    a => p14,
    y => a);


  u2 : inv port map (
    a => p13,
    y => b);


  u3 : rtslice port map (
    a => p10,
    b => p12,
    c => a,
    d => b,
    y => z,
    y2 => i);

  tp6 <= z;
  p11 <= i;

  u4 : rtslice2 port map (
    a => p28,
    b => z,
    c => a,
    d => b,
    y => l,
    y2 => v,
    y3 => p19);

  tp5 <= l;
  p26 <= v;

  u5 : g2 port map (
    a => g,
    b => j,
    y => p20);

  p3 <= g;
  tp1 <= j;

  u6 : g2 port map (
    a => h,
    b => k,
    y => p21);

  p7 <= h;
  tp4 <= k;

  u7 : g2 port map (
    a => i,
    b => l,
    y => p27);


  u8 : rtslice port map (
    a => p6,
    b => p8,
    c => a,
    d => b,
    y => y,
    y2 => h);


  u9 : rtslice2 port map (
    a => p24,
    b => y,
    c => a,
    d => b,
    y => k,
    y2 => u,
    y3 => p16);

  p25 <= u;

  u10 : g2 port map (
    a => x,
    b => t,
    y => p1);

  p22 <= t;
  tp2 <= x;

  u11 : g2 port map (
    a => y,
    b => u,
    y => p5);


  u12 : g2 port map (
    a => z,
    b => v,
    y => p9);


  u13 : rtslice port map (
    a => p2,
    b => p4,
    c => a,
    d => b,
    y => x,
    y2 => g);


  u14 : rtslice2 port map (
    a => p23,
    b => x,
    c => a,
    d => b,
    y => j,
    y2 => t,
    y3 => p15);



end gates;

