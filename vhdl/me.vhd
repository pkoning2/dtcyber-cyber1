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
-- ME module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity me is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p15 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p6_tp1 : out logicsig;
      p7_tp2 : out logicsig;
      p8 : out logicsig;
      p9_tp3 : out logicsig;
      p12_tp4 : out logicsig;
      p13 : out logicsig;
      p19_tp6 : out logicsig;
      p20 : out logicsig;
      p23_tp5 : out logicsig;
      p27 : out logicsig);

end me;
architecture gates of me is
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t18 : logicsig;
  signal t19 : logicsig;
  signal t20 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => c,
    b => p2,
    c => a,
    y => t1);


  u2 : g2 port map (
    a => a,
    b => b,
    y => t2);


  u3 : g2 port map (
    a => c,
    b => b,
    y => t3);


  u4 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => p6_tp1);


  u5 : g3 port map (
    a => c,
    b => d,
    c => f,
    y => t5);


  u6 : g2 port map (
    a => d,
    b => e,
    y => t6);


  u7 : g2 port map (
    a => e,
    b => f,
    y => t7);


  u8 : g3 port map (
    a => t5,
    b => t6,
    c => t7,
    y => p9_tp3);


  u9 : g2 port map (
    a => p2,
    b => b,
    y => t9);


  u10 : g2 port map (
    a => a,
    b => c,
    y => t10);


  u11 : g2 port map (
    a => t9,
    b => t10,
    y => p7_tp2,
    y2 => p8);


  u12 : g2 port map (
    a => c,
    b => e,
    y => t12);


  u13 : g2 port map (
    a => d,
    b => f,
    y => t13);


  u14 : g2 port map (
    a => t12,
    b => t13,
    y => p12_tp4,
    y2 => p13);


  u15 : g2 port map (
    a => f,
    b => h,
    y => t15);


  u16 : g2 port map (
    a => g,
    b => i,
    y => t16);

  p27 <= i;

  u17 : g2 port map (
    a => t15,
    b => t16,
    y => p23_tp5,
    y2 => p20);


  u18 : g3 port map (
    a => f,
    b => i,
    c => g,
    y => t18);


  u19 : g2 port map (
    a => g,
    b => h,
    y => t19);


  u20 : g2 port map (
    a => h,
    b => i,
    y => t20);


  u21 : g3 port map (
    a => t18,
    b => t19,
    c => t20,
    y => p19_tp6);


  u22 : inv port map (
    a => p3,
    y => a);


  u23 : inv port map (
    a => p5,
    y => b);


  u24 : inv port map (
    a => p1,
    y => c);


  u25 : inv port map (
    a => p11,
    y => d);


  u26 : inv port map (
    a => p10,
    y => e);


  u27 : inv port map (
    a => p15,
    y => f);


  u28 : inv port map (
    a => p24,
    y => g);


  u29 : inv port map (
    a => p21,
    y => h);


  u30 : inv port map (
    a => p25,
    y => i);



end gates;

