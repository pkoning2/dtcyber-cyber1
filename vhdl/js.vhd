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
-- JS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity js is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      p2 : out logicsig;
      p6 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23_p25 : out logicsig;
      p28_tp6 : out logicsig);

end js;
architecture gates of js is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
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

  signal a : logicsig;
  signal d : logicsig;
  signal r : logicsig;
  signal t : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
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

begin -- gates
  u1 : g2 port map (
    a => p3,
    b => p4,
    y => t1,
    y2 => t2);

  p6 <= t1;
  p9 <= t2;

  u2 : inv port map (
    a => t2,
    y => t3);


  u3 : rsflop port map (
    r => a,
    s => t1,
    q => tp2,
    qb => p2);

  p10 <= a;

  u4 : rsflop port map (
    r => p8,
    s => t3,
    q => t4);

  tp1 <= t4;

  u5 : inv2 port map (
    a => p5,
    y => t5,
    y2 => t6);


  u7 : latch port map (
    clk => p5,
    d => t4,
    q => t8);


  u8 : g2 port map (
    a => t8,
    b => p7,
    y => t9);

  tp3 <= t9;

  u9 : rsflop port map (
    r => a,
    s => t9,
    q => d);


  u10 : inv port map (
    a => p11,
    y => t10);


  u11 : g2 port map (
    a => d,
    b => t10,
    y2 => t11);


  u12 : g5 port map (
    a => p14,
    b => p18,
    c => t11,
    d => p16,
    e => a,
    y => t12);


  u13 : g5 port map (
    a => a,
    b => t11,
    c => p12,
    d => p15,
    e => p13,
    y => r,
    y2 => t13);


  u14 : g2 port map (
    a => t12,
    b => r,
    y => t);


  u15 : g2 port map (
    a => p19,
    b => t,
    y => p20);


  u16 : g2 port map (
    a => p26,
    b => t,
    y => p21);


  u17 : g2 port map (
    a => t,
    b => p24,
    y => p22);


  u18 : inv port map (
    a => p27,
    y => t14);


  u19 : g2 port map (
    a => t13,
    b => t14,
    y => t15);


  u20 : inv port map (
    a => p17,
    y => t16);


  u21 : g3 port map (
    a => t,
    b => t16,
    c => r,
    y => t17);


  u22 : g2 port map (
    a => t15,
    b => t17,
    y => p28_tp6,
    y2 => p23_p25);


  u23 : inv port map (
    a => p1,
    y => a);



end gates;

