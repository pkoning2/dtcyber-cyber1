-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- HS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hs is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p1 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19_tp6 : out logicsig;
      p20_tp5 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end hs;
architecture gates of hs is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal d : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal r : logicsig;
  signal t : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => b,
    b => d,
    y => t1);


  u2 : g2 port map (
    a => r,
    b => t1,
    y => t2);


  u3 : g3 port map (
    a => r,
    b => g,
    c => p11,
    y => t3,
    y2 => p13);

  p23 <= g;

  u4 : g2 port map (
    a => t2,
    b => t3,
    y => p15);


  u5 : g2 port map (
    a => p11,
    b => n,
    y => tp3,
    y2 => p9);


  u6 : inv port map (
    a => p4,
    y => r);


  u7 : inv port map (
    a => p10,
    y => t);


  u8 : g2 port map (
    a => k,
    b => r,
    y => tp1,
    y2 => p7);

  p1 <= k;

  u9 : inv2 port map (
    a => p26,
    y => a,
    y2 => b);


  u10 : inv2 port map (
    a => p27,
    y => p28,
    y2 => d);


  u11 : inv2 port map (
    a => p24,
    y => p25,
    y2 => f);


  u12 : inv2 port map (
    a => p22,
    y => g,
    y2 => h);


  u13 : inv2 port map (
    a => p21,
    y => i,
    y2 => j);


  u14 : inv2 port map (
    a => p3,
    y => k,
    y2 => l);


  u15 : inv port map (
    a => p2,
    y => m);

  p5 <= m;

  u16 : inv port map (
    a => p12,
    y => n);


  u17 : g3 port map (
    a => t,
    b => r,
    c => n,
    y => tp2,
    y2 => p8);


  u18 : g2 port map (
    a => r,
    b => m,
    y2 => p6);


  u19 : g2 port map (
    a => b,
    b => f,
    y => t5);


  u20 : g2 port map (
    a => r,
    b => t5,
    y2 => p14);


  u21 : g2 port map (
    a => f,
    b => h,
    y => t6);

  p18 <= t6;

  u22 : g3 port map (
    a => r,
    b => t6,
    c => t,
    y => tp4,
    y2 => p17);


  u23 : g2 port map (
    a => j,
    b => l,
    y => t7);

  p19_tp6 <= t7;

  u24 : g2 port map (
    a => t7,
    b => r);


  u25 : g2 port map (
    a => t7,
    b => r,
    y2 => p16);


  u26 : g3 port map (
    a => b,
    b => f,
    c => j,
    y => p20_tp5);



end gates;

