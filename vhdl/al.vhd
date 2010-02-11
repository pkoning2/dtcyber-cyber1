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
-- AL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity al is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p10 : out logicsig;
      p17 : out logicsig;
      p23_tp4 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end al;
architecture gates of al is
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

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
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
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;
  signal t18 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p18,
    y => e);


  u2 : inv port map (
    a => p27,
    y => t1);


  u3 : r2sflop port map (
    r => t1,
    r2 => d,
    s => e,
    q => t2,
    qb => t3);

  tp5 <= t2;

  u4 : g2 port map (
    a => t2,
    b => p28,
    y => t4);


  u5 : rsflop port map (
    r => t5,
    s => t4,
    q => t6);

  tp6 <= t6;

  u6 : inv port map (
    a => t3,
    y => t5);


  u7 : g2 port map (
    a => p13,
    b => p16,
    y => t7);


  u8 : r2sflop port map (
    r => d,
    r2 => p12,
    s => t7,
    q => b);

  tp2 <= b;

  u9 : rsflop port map (
    r => p12,
    s => p11,
    q => t8);

  tp3 <= t8;

  u10 : inv port map (
    a => t8,
    y => t9);

  p24 <= t9;

  u11 : inv port map (
    a => p22,
    y => t10);


  u12 : g4 port map (
    a => t6,
    b => t9,
    c => c,
    d => t10,
    y => t11,
    y2 => p26);

  p25 <= t11;

  u13 : r4s4flop port map (
    r => p19,
    r2 => '1',
    r3 => '1',
    r4 => '1',
    s => p20,
    s2 => t11,
    s3 => p21,
    s4 => '1',
    q => p23_tp4);


  u14 : inv port map (
    a => p9,
    y => c);


  u15 : g3 port map (
    a => t10,
    b => t8,
    c => c,
    y => t13);

  p10 <= t13;

  u16 : g2 port map (
    a => t13,
    b => p8,
    y => p7);


  u17 : g2 port map (
    a => p16,
    b => p14,
    y => t14);


  u18 : rsflop port map (
    r => d,
    s => t14,
    q => t15);

  tp1 <= t15;

  u19 : g2 port map (
    a => t15,
    b => b,
    y => t16);


  u20 : g4 port map (
    a => p4,
    b => p2,
    c => t16,
    d => p3,
    y => p6);


  u21 : g2 port map (
    a => t16,
    b => p3,
    y2 => t17);


  u22 : g2 port map (
    a => p1,
    b => t17,
    y2 => d);


  u23 : inv port map (
    a => p15,
    y => t18);


  u24 : rsflop port map (
    r => t18,
    s => e,
    q => p17);



end gates;

