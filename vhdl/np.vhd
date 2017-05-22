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
-- NP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity np is
    port (
      p2 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p5_tp2 : out logicsig;
      p6_tp1 : out logicsig;
      p7 : out logicsig;
      p8_tp3 : out logicsig;
      p22_tp5 : out logicsig;
      p23_tp6 : out logicsig;
      p24 : out logicsig);

end np;
architecture gates of np is
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
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p27,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p21,
    y => c,
    y2 => d);


  u3 : inv2 port map (
    a => p26,
    y => e,
    y2 => f);


  u4 : inv2 port map (
    a => p25,
    y => g,
    y2 => h);


  u5 : g3 port map (
    a => b,
    b => d,
    c => f,
    y2 => p22_tp5);


  u6 : g2 port map (
    a => b,
    b => h,
    y => l);


  u7 : g3 port map (
    a => c,
    b => l,
    c => e,
    y2 => p23_tp6);


  u8 : g2 port map (
    a => a,
    b => g,
    y => k,
    y2 => p24);


  u9 : g2 port map (
    a => k,
    b => l,
    y => n,
    y2 => m);


  u10 : g2 port map (
    a => p2,
    b => n,
    y => p1);


  u11 : inv port map (
    a => p2,
    y => j);


  u12 : g2 port map (
    a => j,
    b => m,
    y => p4);


  u13 : g2 port map (
    a => j,
    b => k,
    y => t10);


  u14 : g2 port map (
    a => t10,
    b => l,
    y => t11,
    y2 => t12);


  u15 : g2 port map (
    a => t11,
    b => c,
    y => p6_tp1);


  u16 : g2 port map (
    a => t12,
    b => d,
    y => p7);


  u17 : g2 port map (
    a => t12,
    b => c,
    y => t13,
    y2 => t14);


  u18 : g2 port map (
    a => t13,
    b => e,
    y => p5_tp2);


  u19 : g2 port map (
    a => t14,
    b => f,
    y => p8_tp3);



end gates;

