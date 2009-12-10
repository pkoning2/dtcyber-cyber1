-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- MJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mj is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end mj;
architecture gates of mj is
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

  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
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
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;
  signal t18 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p11,
    y => t1);


  u2 : g2 port map (
    a => a,
    b => p17,
    y => t2);


  u3 : g3 port map (
    a => a,
    b => b,
    c => p8,
    y => t3);


  u4 : g3 port map (
    a => a,
    b => c,
    c => p9,
    y => t4);


  u5 : g4 port map (
    a => a,
    b => b,
    c => p10,
    d => d,
    y => t5);


  u6 : g4 port map (
    a => c,
    b => p12,
    c => p6,
    d => a,
    y => t6);


  u7 : g6 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    e => t5,
    f => t6,
    y => t7,
    y2 => t8);

  tp2 <= t7;

  u8 : inv port map (
    a => t8,
    y => t9);

  p2 <= t9;
  p15 <= t9;
  tp1 <= t9;

  u9 : g2 port map (
    a => t7,
    b => p7,
    y => p14);


  u10 : g3 port map (
    a => p7,
    b => t7,
    c => p3,
    y => p1);


  u11 : g4 port map (
    a => b,
    b => p11,
    c => d,
    d => p19,
    y => t10);


  u12 : inv port map (
    a => p17,
    y => t11);


  u13 : g2 port map (
    a => p8,
    b => b,
    y => t12);


  u14 : g2 port map (
    a => p9,
    b => c,
    y => t13);


  u15 : g3 port map (
    a => p10,
    b => b,
    c => d,
    y => t14);


  u16 : g3 port map (
    a => p12,
    b => p6,
    c => c,
    y => t15);


  u17 : g6 port map (
    a => t10,
    b => t11,
    c => t12,
    d => t13,
    e => t14,
    f => t15,
    y => t16,
    y2 => t17);

  tp5 <= t16;

  u18 : g3 port map (
    a => p18,
    b => t16,
    c => p16,
    y => p28);


  u19 : g2 port map (
    a => p16,
    b => t16,
    y => p26);


  u20 : inv port map (
    a => t17,
    y => t18);

  p22 <= t18;
  p24 <= t18;
  tp6 <= t18;

  u21 : inv port map (
    a => p5,
    y => a);


  u22 : inv port map (
    a => p21,
    y => b);


  u23 : inv port map (
    a => p20,
    y => c);


  u24 : inv port map (
    a => p27,
    y => d);



end gates;

