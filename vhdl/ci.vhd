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
-- CI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ci is
    port (
      p2 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p8_tp5 : out logicsig;
      p12_tp6 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p20_p22 : out logicsig;
      p21_p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ci;
architecture gates of ci is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p7,
    y => d);


  u2 : inv port map (
    a => p6,
    y => c);


  u3 : g3 port map (
    a => p7,
    b => p10,
    c => p6,
    y => t1,
    y2 => p1);


  u4 : g2 port map (
    a => c,
    b => d,
    y => t2);


  u7 : inv port map (
    a => t4,
    y => p3);

  tp1 <= t4;

  u10 : inv port map (
    a => t6,
    y => p4);

  tp2 <= t6;

  u11 : g3 port map (
    a => t1,
    b => t2,
    c => p5,
    y => t10);


  u12 : g3 port map (
    a => p5,
    b => c,
    c => d,
    y => t11);


  u13 : latch port map (
    clk => p2,
    d => t10,
    q => t4);


  u14 : latch port map (
    clk => p2,
    d => t11,
    q => t6);


  u15 : inv port map (
    a => p27,
    y => t13);


  u16 : inv port map (
    a => p25,
    y => t14);


  u17 : g2 port map (
    a => t13,
    b => t14,
    y => p28);


  u18 : inv port map (
    a => p13,
    y => t15);


  u19 : g2 port map (
    a => p11,
    b => t15,
    y => p12_tp6);


  u20 : g2 port map (
    a => p16,
    b => t15,
    y => p14);


  u21 : g2 port map (
    a => p17,
    b => t15,
    y => p15);


  u22 : g2 port map (
    a => t15,
    b => p9,
    y => p8_tp5);


  u23 : inv2 port map (
    a => p23,
    y => p26,
    y2 => p21_p24);


  u24 : inv2 port map (
    a => p19,
    y => p18,
    y2 => p20_p22);



end gates;

