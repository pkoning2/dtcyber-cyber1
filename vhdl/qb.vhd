-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- QB module, rev E
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qb is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p2_tp1 : out logicsig;
      p4_tp2 : out logicsig;
      p6_tp4 : out logicsig;
      p8_tp3 : out logicsig;
      p12 : out logicsig;
      p14_p16 : out logicsig;
      p18 : out logicsig;
      p20_p22_p24 : out logicsig;
      p26_tp6 : out logicsig;
      p28_tp5 : out logicsig);

end qb;
architecture gates of qb is
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
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t16 : logicsig;
  signal t17 : logicsig;
  signal t18 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p7,
    b => p5,
    y => c);


  u2 : inv2 port map (
    a => p3,
    y => f,
    y2 => t1);


  u3 : g2 port map (
    a => p17,
    b => p19,
    y => b);


  u4 : inv2 port map (
    a => p15,
    y => e,
    y2 => t2);


  u5 : g2 port map (
    a => p11,
    b => p9,
    y => a);


  u6 : inv2 port map (
    a => p13,
    y => d,
    y2 => t3);


  u7 : g2 port map (
    a => c,
    b => t1,
    y => p4_tp2);


  u8 : g2 port map (
    a => b,
    b => t2,
    y => p8_tp3);


  u9 : g2 port map (
    a => a,
    b => t3,
    y => p6_tp4);


  u10 : g3 port map (
    a => p21,
    b => p25,
    c => p27,
    y => x);


  u11 : inv2 port map (
    a => x,
    y => p18,
    y2 => p26_tp6);


  u12 : g2 port map (
    a => a,
    b => x,
    y => t8);


  u13 : inv port map (
    a => d,
    y => t9);


  u14 : g2 port map (
    a => t8,
    b => t9,
    y => p2_tp1,
    y2 => p1);


  u15 : g3 port map (
    a => a,
    b => b,
    c => c,
    y2 => p20_p22_p24);


  u16 : g2 port map (
    a => e,
    b => c,
    y => t12);


  u17 : inv port map (
    a => f,
    y => t13);


  u18 : g3 port map (
    a => c,
    b => b,
    c => d,
    y => t14);


  u19 : g3 port map (
    a => t12,
    b => t13,
    c => t14,
    y => p14_p16);


  u20 : g3 port map (
    a => x,
    b => a,
    c => b,
    y => t16);


  u21 : g2 port map (
    a => d,
    b => b,
    y => t17);


  u22 : inv port map (
    a => e,
    y => t18);


  u23 : g3 port map (
    a => t18,
    b => t16,
    c => t17,
    y => p28_tp5,
    y2 => p12);



end gates;

