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
-- KV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kv is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_tp2 : out logicsig;
      p12_p13_p15_p17 : out logicsig;
      p14 : out logicsig;
      p19 : out logicsig;
      p20_p22 : out logicsig);

end kv;
architecture gates of kv is
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
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal j : logicsig;
  signal n : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : g5 port map (
    a => p2,
    b => p4,
    c => p6,
    d => p8,
    e => p10,
    y => tp1,
    y2 => p12_p13_p15_p17);


  u2 : g3 port map (
    a => p3,
    b => p5,
    c => p7,
    y => p1_tp2);


  u3 : g3 port map (
    a => p24,
    b => p26,
    c => p28,
    y => tp4,
    y2 => p20_p22);


  u4 : inv2 port map (
    a => p23,
    y => a,
    y2 => e);


  u5 : inv2 port map (
    a => p21,
    y => b,
    y2 => f);


  u6 : inv2 port map (
    a => p11,
    y => t1,
    y2 => g);


  u7 : inv2 port map (
    a => p9,
    y => t2,
    y2 => h);


  u8 : g2 port map (
    a => t1,
    b => t2,
    y => j);


  u9 : inv port map (
    a => p18,
    y => n);


  u10 : g2 port map (
    a => p18,
    b => p16,
    y2 => r);


  u11 : g2 port map (
    a => e,
    b => n,
    y => t3);


  u12 : g3 port map (
    a => a,
    b => f,
    c => r,
    y => t4);


  u13 : g4 port map (
    a => a,
    b => g,
    c => h,
    d => r,
    y => t5);


  u14 : g4 port map (
    a => e,
    b => f,
    c => j,
    d => r,
    y => t6);

  tp3 <= t6;

  u15 : g4 port map (
    a => t3,
    b => t4,
    c => t5,
    d => t6,
    y => tp6,
    y2 => p14);


  u16 : g2 port map (
    a => f,
    b => n,
    y => t7);


  u17 : g3 port map (
    a => b,
    b => e,
    c => r,
    y => t8);


  u18 : g4 port map (
    a => b,
    b => g,
    c => h,
    d => r,
    y => t9);


  u19 : g4 port map (
    a => t6,
    b => t7,
    c => t8,
    d => t9,
    y => tp5,
    y2 => p19);



end gates;

