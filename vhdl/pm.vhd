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
-- PM module, rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pm is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p17 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p1_tp1 : out logicsig;
      p2 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11_tp2 : out logicsig;
      p12_p14 : out logicsig;
      p15_tp5 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p19_p20 : out logicsig;
      p27_tp6 : out logicsig;
      p28 : out logicsig);

end pm;
architecture gates of pm is
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
  signal n : logicsig;
  signal r : logicsig;
  signal t : logicsig;

begin -- gates
  u1 : inv port map (
    a => p17,
    y => j);


  u2 : inv port map (
    a => p13,
    y => k);


  u3 : inv port map (
    a => p23,
    y => l);


  u4 : inv port map (
    a => p21,
    y => m);


  u5 : inv port map (
    a => p3,
    y => n);


  u6 : inv port map (
    a => p5,
    y => r);


  u7 : inv port map (
    a => p25,
    y => t);


  u8 : g3 port map (
    a => j,
    b => p8,
    c => r,
    y => e);


  u9 : g3 port map (
    a => k,
    b => p6,
    c => r,
    y => f);


  u10 : g2 port map (
    a => e,
    b => f,
    y => p1_tp1,
    y2 => p2);


  u11 : inv2 port map (
    a => f,
    y2 => p7);


  u12 : g4 port map (
    a => a,
    b => c,
    c => e,
    d => g,
    y2 => p19_p20);


  u13 : g3 port map (
    a => j,
    b => p10,
    c => n,
    y => c);


  u14 : g3 port map (
    a => k,
    b => p4,
    c => n,
    y => d);


  u15 : g2 port map (
    a => c,
    b => d,
    y => p11_tp2,
    y2 => p16);


  u16 : inv2 port map (
    a => d,
    y2 => p9);


  u17 : g4 port map (
    a => b,
    b => d,
    c => f,
    d => h,
    y2 => p12_p14);


  u18 : g3 port map (
    a => k,
    b => p26,
    c => t,
    y => h);


  u19 : g3 port map (
    a => t,
    b => j,
    c => p24,
    y => g);


  u20 : g2 port map (
    a => h,
    b => g,
    y => p15_tp5,
    y2 => p18);


  u21 : g4 port map (
    a => p24,
    b => k,
    c => l,
    d => m,
    y => b);


  u22 : g4 port map (
    a => j,
    b => l,
    c => m,
    d => p22,
    y => a);


  u23 : g2 port map (
    a => b,
    b => a,
    y => p27_tp6,
    y2 => p28);



end gates;

