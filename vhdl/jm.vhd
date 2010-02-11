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
-- JM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jm is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p14 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9_tp2 : out logicsig;
      p10_tp1 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p21_p25 : out logicsig;
      p23 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end jm;
architecture gates of jm is
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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal n : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;
  signal y : logicsig;
  signal z : logicsig;

begin -- gates
  u1 : g4 port map (
    a => l,
    b => j,
    c => h,
    d => n,
    y => p16);

  p15 <= l;
  p17 <= h;
  p21_p25 <= n;
  p23 <= j;

  u2 : g4 port map (
    a => l,
    b => j,
    c => g,
    d => n,
    y => p20,
    y2 => t1);


  u3 : g2 port map (
    a => e,
    b => t1,
    y => p9_tp2);


  u4 : g3 port map (
    a => t1,
    b => p14,
    c => e,
    y2 => r);


  u5 : g3 port map (
    a => r,
    b => b,
    c => d,
    y => p5);

  p3 <= b;
  p12 <= d;

  u6 : g4 port map (
    a => l,
    b => i,
    c => h,
    d => n,
    y => p18);


  u7 : g4 port map (
    a => l,
    b => i,
    c => g,
    d => n,
    y => y,
    y2 => p27);


  u8 : g4 port map (
    a => k,
    b => i,
    c => g,
    d => n,
    y => z,
    y2 => p28);


  u9 : g3 port map (
    a => a,
    b => c,
    c => e,
    y => p13,
    y2 => p11);


  u10 : g3 port map (
    a => r,
    b => a,
    c => d,
    y => p10_tp1);


  u11 : g3 port map (
    a => r,
    b => b,
    c => c,
    y => p2);


  u12 : g3 port map (
    a => r,
    b => a,
    c => c,
    y => p1);


  u13 : inv2 port map (
    a => p22,
    y => k,
    y2 => l);


  u14 : inv2 port map (
    a => p26,
    y => i,
    y2 => j);


  u15 : inv2 port map (
    a => p24,
    y => g,
    y2 => h);


  u16 : inv2 port map (
    a => p19,
    y => tp6,
    y2 => n);


  u17 : inv2 port map (
    a => p4,
    y => e,
    y2 => p7);


  u18 : inv2 port map (
    a => p6,
    y => c,
    y2 => d);


  u19 : inv2 port map (
    a => p8,
    y => a,
    y2 => b);



end gates;

