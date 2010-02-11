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
-- HL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hl is
    port (
      p1 : in  logicsig;
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
      p12 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp6 : out logicsig;
      p13_tp5 : out logicsig;
      p15 : out logicsig;
      p16_tp4 : out logicsig);

end hl;
architecture gates of hl is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;
  signal t20 : logicsig;
  signal t21 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p2,
    y => t1);


  u2 : inv port map (
    a => p1,
    y => t2);


  u3 : inv port map (
    a => p7,
    y => t3);


  u4 : inv port map (
    a => p8,
    y => t4);


  u5 : inv port map (
    a => p27,
    y => t5);


  u6 : inv port map (
    a => p28,
    y => t6);


  u7 : inv port map (
    a => p22,
    y => t7);


  u8 : inv port map (
    a => p21,
    y => t8);


  u9 : inv port map (
    a => p20,
    y => a);


  u10 : inv2 port map (
    a => p18,
    y => p16_tp4,
    y2 => p15);


  u11 : g3 port map (
    a => p3,
    b => t1,
    c => a,
    y => t10);


  u12 : g3 port map (
    a => p4,
    b => t2,
    c => a,
    y => t11);


  u13 : g3 port map (
    a => p6,
    b => t3,
    c => a,
    y => t12);


  u14 : g3 port map (
    a => p5,
    b => t4,
    c => a,
    y => t13);


  u15 : g3 port map (
    a => p26,
    b => t5,
    c => a,
    y => t14);


  u16 : g3 port map (
    a => p25,
    b => t6,
    c => a,
    y => t15);


  u17 : g3 port map (
    a => p23,
    b => t7,
    c => a,
    y => t16);


  u18 : g3 port map (
    a => p24,
    b => t8,
    c => a,
    y => t17);


  u19 : g5 port map (
    a => t10,
    b => t11,
    c => t12,
    d => t13,
    e => p10,
    y => tp1,
    y2 => t20);


  u20 : g6 port map (
    a => t14,
    b => t15,
    c => t16,
    d => t17,
    e => p12,
    f => p14,
    y => tp6,
    y2 => t21);


  u21 : g6 port map (
    a => t20,
    b => p9,
    c => p11,
    d => p17,
    e => p19,
    f => t21,
    y => p13_tp5);



end gates;

