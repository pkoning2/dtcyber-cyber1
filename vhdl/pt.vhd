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
-- PT module, rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pt is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p9_p11 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig);

end pt;
architecture gates of pt is
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
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal p : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : g4 port map (
    a => a,
    b => j,
    c => c,
    d => b,
    y => t1);


  u2 : g2 port map (
    a => d,
    b => a,
    y => t2);


  u3 : g2 port map (
    a => t1,
    b => t2,
    y => t3);

  tp1 <= t3;

  u4 : g2 port map (
    a => k,
    b => t3,
    y => p9_p11);


  u5 : g2 port map (
    a => t3,
    b => p4,
    y => t5);

  tp2 <= t5;

  u6 : g2 port map (
    a => t5,
    b => k,
    y => p17);


  u7 : inv2 port map (
    a => p1,
    y => h,
    y2 => b);


  u8 : inv2 port map (
    a => p3,
    y => i,
    y2 => c);


  u9 : inv2 port map (
    a => p5,
    y => j,
    y2 => d);


  u10 : inv2 port map (
    a => p27,
    y => n,
    y2 => e);


  u11 : inv2 port map (
    a => p25,
    y => m,
    y2 => g);


  u12 : inv port map (
    a => p23,
    y => l);


  u13 : inv2 port map (
    a => p7,
    y => p,
    y2 => k);


  u14 : g3 port map (
    a => l,
    b => m,
    c => n,
    y => tp5,
    y2 => a);


  u15 : g4 port map (
    a => a,
    b => j,
    c => c,
    d => h,
    y => t6);


  u16 : g4 port map (
    a => a,
    b => j,
    c => i,
    d => b,
    y => t7);


  u17 : g3 port map (
    a => g,
    b => l,
    c => e,
    y => t8);


  u18 : g3 port map (
    a => t6,
    b => t7,
    c => t8,
    y => t9,
    y2 => p15);

  tp6 <= t9;

  u19 : inv port map (
    a => p21,
    y => t10);


  u20 : g3 port map (
    a => t10,
    b => t9,
    c => k,
    y => p19);



end gates;

