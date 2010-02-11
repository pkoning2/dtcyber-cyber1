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
-- CG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cg is
    port (
      p4 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      p2 : out logicsig;
      p3_p5 : out logicsig;
      p6_tp2 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23_tp6 : out logicsig;
      p24 : out logicsig;
      p26_p28 : out logicsig;
      p27 : out logicsig);

end cg;
architecture gates of cg is
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

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
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

begin -- gates
  u1 : g2 port map (
    a => d,
    b => p4,
    y => p2,
    y2 => t1);

  p7 <= d;

  u2 : inv2 port map (
    a => p14,
    y2 => b);


  u3 : inv2 port map (
    a => p10,
    y2 => e);


  u4 : g2 port map (
    a => p19,
    b => p21,
    y => t2);


  u5 : inv port map (
    a => p13,
    y => t3);


  u6 : g5 port map (
    a => p10,
    b => t2,
    c => p16,
    d => t3,
    e => p12,
    y => f);


  u7 : g2 port map (
    a => t1,
    b => b,
    y => p9,
    y2 => t5);


  u8 : inv port map (
    a => t5,
    y => p3_p5);


  u9 : latch port map (
    clk => p14,
    d => t1,
    q => t7,
    qb => tp1);


  u10 : inv port map (
    a => p11,
    y => t8);


  u11 : g3 port map (
    a => e,
    b => t7,
    c => t8,
    y => c);

  p6_tp2 <= c;

  u12 : rsflop port map (
    r => f,
    s => c,
    q => t9,
    qb => h);


  u13 : inv port map (
    a => t9,
    y => g);

  p24 <= g;

  u14 : g2 port map (
    a => g,
    b => p25,
    y => p27,
    y2 => t10);


  u15 : g2 port map (
    a => t10,
    b => b,
    y => p20,
    y2 => t11);


  u16 : inv port map (
    a => t11,
    y => p26_p28);


  u17 : latch port map (
    clk => p14,
    d => t10,
    q => t13);


  u18 : inv port map (
    a => p18,
    y => t14);


  u19 : g3 port map (
    a => e,
    b => t13,
    c => t14,
    y => t15);

  p23_tp6 <= t15;

  u20 : rsflop port map (
    r => f,
    s => t15,
    q => t16,
    qb => t17);


  u21 : inv port map (
    a => t16,
    y => d);


  u22 : g3 port map (
    a => h,
    b => t17,
    c => p15,
    y => p22);



end gates;

