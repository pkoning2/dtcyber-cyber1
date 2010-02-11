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
-- IB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ib is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      p4_tp1 : out logicsig;
      p6 : out logicsig;
      p13_p15_p17_p19 : out logicsig;
      p21_tp6 : out logicsig;
      p23 : out logicsig;
      p27 : out logicsig);

end ib;
architecture gates of ib is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component r4s4flop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      r3 : in  logicsig;
      r4 : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => d,
    y => t1);

  tp2 <= a;

  u2 : g2 port map (
    a => p1,
    b => p3,
    y => t2);


  u3 : g2 port map (
    a => f,
    b => t2,
    y => t3);


  u4 : r4s4flop port map (
    r => e,
    r2 => c,
    r3 => '1',
    r4 => '1',
    s => t1,
    s2 => t3,
    s3 => '1',
    s4 => '1',
    q => t4);

  p4_tp1 <= t4;

  u5 : g2 port map (
    a => t4,
    b => p10,
    y => p6);


  u6 : g2 port map (
    a => b,
    b => d,
    y => t5);

  tp5 <= b;

  u7 : g2 port map (
    a => p28,
    b => p26,
    y => t6);


  u8 : g2 port map (
    a => f,
    b => t6,
    y => t7);


  u9 : r4s4flop port map (
    r => c,
    r2 => e,
    r3 => '1',
    r4 => '1',
    s => t5,
    s2 => t7,
    s3 => '1',
    s4 => '1',
    q => t8,
    qb => p23);

  p21_tp6 <= t8;

  u10 : g2 port map (
    a => p24,
    b => t8,
    y => p27);


  u11 : rs2flop port map (
    r => t11,
    s => p11,
    s2 => p9,
    q => a);


  u13 : inv2 port map (
    a => p8,
    y => t11,
    y2 => p13_p15_p17_p19);


  u14 : rs2flop port map (
    r => t11,
    s => p20,
    s2 => p18,
    q => b);


  u15 : inv2 port map (
    a => p7,
    y => c,
    y2 => d);


  u16 : inv2 port map (
    a => p22,
    y => e,
    y2 => f);



end gates;

