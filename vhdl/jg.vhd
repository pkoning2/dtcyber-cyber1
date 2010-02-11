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
-- JG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jg is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p1_tp1 : out logicsig;
      p10_tp2 : out logicsig;
      p17_tp5 : out logicsig;
      p28_tp6 : out logicsig);

end jg;
architecture gates of jg is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p6,
    b => p4,
    y => t1);


  u2 : inv port map (
    a => p4,
    y => t2);


  u3 : g2 port map (
    a => t2,
    b => p7,
    y => c);


  u4 : g2 port map (
    a => a,
    b => c,
    y => t3);


  u5 : g3 port map (
    a => b,
    b => c,
    c => d,
    y => t4);


  u6 : g3 port map (
    a => t1,
    b => t3,
    c => t4,
    y => p1_tp1);


  u7 : g2 port map (
    a => p21,
    b => p23,
    y => t6);


  u8 : inv port map (
    a => p23);


  u9 : inv port map (
    a => p23,
    y => t7);


  u10 : g2 port map (
    a => p22,
    b => t7,
    y => h);


  u11 : g2 port map (
    a => f,
    b => h,
    y => t8);


  u12 : g3 port map (
    a => g,
    b => h,
    c => i,
    y => t9);


  u13 : g3 port map (
    a => t6,
    b => t8,
    c => t9,
    y => p28_tp6);


  u14 : g2 port map (
    a => p14,
    b => p20,
    y2 => f);


  u15 : inv port map (
    a => p20,
    y => t11);


  u16 : g2 port map (
    a => t11,
    b => p19,
    y => i);


  u17 : g2 port map (
    a => p9,
    b => p11,
    y2 => a);


  u18 : inv port map (
    a => p11,
    y => t12);


  u19 : g2 port map (
    a => p8,
    b => t12,
    y2 => d);


  u20 : g2 port map (
    a => p26,
    b => p24,
    y2 => g);


  u21 : inv port map (
    a => p24,
    y => t13);


  u22 : g2 port map (
    a => t13,
    b => p25,
    y => j);


  u23 : g2 port map (
    a => p3,
    b => p5,
    y2 => b);


  u24 : inv port map (
    a => p5,
    y => t14);


  u25 : g2 port map (
    a => t14,
    b => p2,
    y => e);


  u26 : g3 port map (
    a => c,
    b => d,
    c => e,
    y => p10_tp2);


  u27 : g3 port map (
    a => h,
    b => i,
    c => j,
    y => p17_tp5);



end gates;

