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
-- NO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity no is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      p1_p12_p25_tp3 : out logicsig;
      p7 : out logicsig;
      p8_p10 : out logicsig;
      p9 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16_tp4 : out logicsig;
      p18_p23 : out logicsig;
      p20_p22 : out logicsig;
      p21_tp6 : out logicsig;
      p24_p26_p28 : out logicsig);

end no;
architecture gates of no is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal i : logicsig;
  signal n : logicsig;
  signal o : logicsig;
  signal p : logicsig;
  signal q : logicsig;
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
  signal t10 : logicsig;

begin -- gates
  u1 : rsflop port map (
    r => i,
    s => p6,
    q => a,
    qb => b);

  tp1 <= a;

  u2 : rsflop port map (
    r => i,
    s => p2,
    q => c,
    qb => d);

  tp2 <= c;

  u3 : g2 port map (
    a => p13,
    b => b,
    y => e);


  u4 : g2 port map (
    a => p11,
    b => d,
    y => t1);


  u5 : g2 port map (
    a => e,
    b => t1,
    y => t2);

  p1_p12_p25_tp3 <= t2;

  u6 : g2 port map (
    a => t2,
    b => n,
    y => t3);


  u7 : g2 port map (
    a => t3,
    b => n,
    y => p8_p10);


  u8 : g2 port map (
    a => p3,
    b => t2,
    y => p9);


  u9 : inv port map (
    a => p3,
    y => t4);


  u11 : inv2 port map (
    a => p3,
    y2 => p7);


  u12 : g2 port map (
    a => p11,
    b => c,
    y => t5);


  u13 : g2 port map (
    a => p13,
    b => a,
    y => t6);


  u14 : g2 port map (
    a => t5,
    b => t6,
    y => t7);

  tp5 <= t7;

  u15 : g2 port map (
    a => t7,
    b => o,
    y => p16_tp4);

  p15 <= o;

  u16 : g2 port map (
    a => t7,
    b => p,
    y => t8);


  u17 : g2 port map (
    a => t8,
    b => p,
    y => p14);


  u18 : g2 port map (
    a => t7,
    b => r,
    y => p21_tp6);

  p18_p23 <= r;

  u19 : g2 port map (
    a => t7,
    b => q,
    y => t9);


  u20 : g2 port map (
    a => t9,
    b => q,
    y2 => p20_p22);


  u21 : g2 port map (
    a => t7,
    b => p27,
    y => t10);


  u22 : g2 port map (
    a => t10,
    b => p27,
    y2 => p24_p26_p28);


  u23 : inv port map (
    a => p4,
    y => i);


  u24 : inv2 port map (
    a => p5,
    y2 => n);


  u25 : inv2 port map (
    a => p19,
    y => q,
    y2 => r);


  u26 : inv2 port map (
    a => p17,
    y => o,
    y2 => p);



end gates;

