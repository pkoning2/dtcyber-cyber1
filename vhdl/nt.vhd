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
-- NT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ntslice1 is
    port (
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      l : in  logicsig;
      q : in  logicsig;
      tp : out logicsig;
      a : out logicsig;
      e : out logicsig;
      y : out logicsig);

end ntslice1;
architecture gates of ntslice1 is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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

begin -- gates
  u1 : inv port map (
    a => i1,
    y => t1);

  e <= t1;

  u2 : inv port map (
    a => i2,
    y => t2);


  u3 : g2 port map (
    a => t1,
    b => t2,
    y => a);


  u4 : g4 port map (
    a => q,
    b => l,
    c => d,
    d => i2,
    y => tp,
    y2 => y);



end gates;

use work.sigs.all;

entity ntslice2 is
    port (
      a : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      j : out logicsig;
      p : out logicsig);

end ntslice2;
architecture gates of ntslice2 is
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

  signal t1 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : inv port map (
    a => a,
    y => t1);


  u2 : g2 port map (
    a => f,
    b => t1,
    y => j,
    y2 => t3);


  u3 : g2 port map (
    a => g,
    b => t3,
    y => p);



end gates;

use work.sigs.all;

entity nt is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p8 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p23 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end nt;
architecture gates of nt is
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

  component ntslice1
    port (
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      l : in  logicsig;
      q : in  logicsig;
      tp : out logicsig;
      a : out logicsig;
      e : out logicsig;
      y : out logicsig);

  end component;

  component ntslice2
    port (
      a : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      j : out logicsig;
      p : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal p : logicsig;
  signal q : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p25,
    y => t1);


  u2 : ntslice1 port map (
    d => d,
    i1 => t1,
    i2 => p24,
    l => l,
    q => q,
    a => a,
    e => e,
    tp => tp6,
    y => p23);

  tp5 <= q;

  u3 : ntslice1 port map (
    d => a,
    i1 => p6,
    i2 => p20,
    l => m,
    q => r,
    a => b,
    e => f,
    y => p18);

  tp1 <= m;
  tp2 <= b;
  tp3 <= f;

  u4 : ntslice1 port map (
    d => j,
    i1 => p21,
    i2 => p7,
    l => s,
    q => b,
    a => c,
    e => g,
    y => p8);

  tp4 <= g;

  u5 : inv port map (
    a => p2,
    y => t2);


  u6 : ntslice1 port map (
    d => p,
    i1 => t2,
    i2 => t3,
    l => k,
    q => c,
    a => d,
    e => h,
    y => p19);


  u7 : inv port map (
    a => p4,
    y => t3);


  u8 : ntslice2 port map (
    a => a,
    f => f,
    g => g,
    j => j,
    p => p);


  u9 : ntslice2 port map (
    a => b,
    f => g,
    g => h,
    j => k,
    p => q);


  u10 : ntslice2 port map (
    a => c,
    f => h,
    g => e,
    j => l,
    p => r);


  u11 : ntslice2 port map (
    a => d,
    f => e,
    g => f,
    j => m,
    p => s);


  u12 : inv2 port map (
    a => p27,
    y => p26,
    y2 => p28);



end gates;

