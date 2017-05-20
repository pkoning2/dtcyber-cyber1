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
-- KQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kq is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p15 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p9_tp3 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p17_tp4 : out logicsig;
      p18_tp5 : out logicsig;
      p19_tp6 : out logicsig;
      p21 : out logicsig;
      p26 : out logicsig);

end kq;
architecture gates of kq is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
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
  signal p : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p6,
    y => p5,
    y2 => g);


  u2 : inv port map (
    a => p7,
    y => i);


  u3 : inv port map (
    a => p8,
    y => j);


  u4 : inv2 port map (
    a => p4,
    y => p3,
    y2 => h);


  u5 : inv2 port map (
    a => p2,
    y => t1,
    y2 => p1);


  u6 : g5 port map (
    a => g,
    b => i,
    c => j,
    d => h,
    e => t1,
    y => tp1,
    y2 => d);


  u7 : g3 port map (
    a => g,
    b => h,
    c => i,
    y2 => p);

  tp2 <= p;

  u8 : inv port map (
    a => p27,
    y => a);


  u9 : inv port map (
    a => p25,
    y => b);


  u10 : inv port map (
    a => p15,
    y => c);


  u11 : inv port map (
    a => p28,
    y => e);


  u12 : g3 port map (
    a => a,
    b => c,
    c => d,
    y => t2);


  u13 : g2 port map (
    a => b,
    b => d,
    y => t3);


  u14 : g2 port map (
    a => t2,
    b => t3,
    y => t4,
    y2 => p16);


  u15 : g2 port map (
    a => t4,
    b => p20,
    y => p17_tp4);


  u16 : g3 port map (
    a => p20,
    b => t4,
    c => p22,
    y => p18_tp5);


  u17 : inv port map (
    a => a,
    y => t5);


  u18 : g3 port map (
    a => b,
    b => d,
    c => e,
    y => t6);


  u19 : g2 port map (
    a => t5,
    b => t6,
    y => t7,
    y2 => p19_tp6);


  u20 : g2 port map (
    a => t7,
    b => p23,
    y => p21);


  u21 : g3 port map (
    a => p23,
    b => t7,
    c => p24,
    y => p26);


  u22 : inv port map (
    a => b,
    y => t8);


  u23 : g2 port map (
    a => a,
    b => c,
    y => t9);


  u24 : g2 port map (
    a => t8,
    b => t9,
    y => t10,
    y2 => p14);


  u25 : g2 port map (
    a => g,
    b => t10,
    y => p9_tp3);


  u26 : g3 port map (
    a => g,
    b => h,
    c => t10,
    y => p13);


  u27 : g2 port map (
    a => t10,
    b => p,
    y => p11);


  u28 : g3 port map (
    a => t10,
    b => p,
    c => j,
    y => p12,
    y2 => p10);



end gates;

