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
-- AQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity aq is
    port (
      p1 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4_tp1 : out logicsig;
      p5 : out logicsig;
      p6_tp2 : out logicsig;
      p7 : out logicsig;
      p8_tp3 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p17 : out logicsig;
      p18_tp4 : out logicsig;
      p20_tp5 : out logicsig;
      p21_tp6 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end aq;
architecture gates of aq is
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
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal r : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p14,
    y => b,
    y2 => a);


  u2 : inv2 port map (
    a => p10,
    y => d,
    y2 => c);


  u3 : inv2 port map (
    a => p15,
    y => f,
    y2 => e);


  u4 : g3 port map (
    a => a,
    b => c,
    c => f,
    y2 => p4_tp1);


  u5 : g3 port map (
    a => b,
    b => d,
    c => e,
    y => p13,
    y2 => p6_tp2);


  u6 : inv port map (
    a => p23,
    y => g);


  u7 : inv port map (
    a => p1,
    y => h);


  u8 : inv port map (
    a => p11,
    y => t3);


  u9 : g3 port map (
    a => p1,
    b => g,
    c => t3,
    y => t4);


  u10 : g4 port map (
    a => a,
    b => t4,
    c => c,
    d => e,
    y => n);

  p2 <= n;

  u11 : g4 port map (
    a => m,
    b => n,
    c => r,
    d => p19,
    y => t5);

  p5 <= t5;
  p26 <= m;

  u12 : g2 port map (
    a => l,
    b => p9,
    y => t6);

  p7 <= t6;
  p18_tp4 <= l;

  u13 : g2 port map (
    a => t5,
    b => t6,
    y => p8_tp3);


  u14 : g4 port map (
    a => p11,
    b => h,
    c => g,
    d => p22,
    y => p20_tp5,
    y2 => p3);


  u15 : g2 port map (
    a => p22,
    b => p24,
    y => m);


  u16 : g2 port map (
    a => p19,
    b => m,
    y => p21_tp6);


  u17 : inv port map (
    a => p16,
    y => p12);


  u18 : g2 port map (
    a => b,
    b => d,
    y => t10);

  p17 <= t10;

  u19 : g2 port map (
    a => p16,
    b => t10,
    y => l,
    y2 => r);


  u20 : g2 port map (
    a => r,
    b => n,
    y => p25);



end gates;

