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
-- JD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jd is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5_tp1 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p10_tp2 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p14_tp3 : out logicsig;
      p16 : out logicsig;
      p18_tp4 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23_tp5 : out logicsig;
      p24_tp6 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end jd;
architecture gates of jd is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p26,
    y => a);


  u2 : inv port map (
    a => p3,
    y => f);


  u3 : latch port map (
    clk => p28,
    d => p15,
    q => t1,
    qb => p25);

  p24_tp6 <= t1;

  u4 : g2 port map (
    a => a,
    b => t1,
    y => p27);


  u5 : latch port map (
    clk => p1,
    d => p15,
    q => t2,
    qb => p4);

  p5_tp1 <= t2;

  u6 : g2 port map (
    a => f,
    b => t2,
    y => p2);


  u7 : latch port map (
    clk => p28,
    d => p13,
    q => t3,
    qb => p20);

  p23_tp5 <= t3;

  u8 : g2 port map (
    a => a,
    b => t3,
    y => p22);


  u9 : latch port map (
    clk => p1,
    d => p13,
    q => t4,
    qb => p9);

  p10_tp2 <= t4;

  u10 : g2 port map (
    a => t4,
    b => f,
    y => p7);


  u11 : latch port map (
    clk => p28,
    d => p17,
    q => t5,
    qb => p21);

  p18_tp4 <= t5;

  u12 : g2 port map (
    a => t5,
    b => a,
    y => p16);


  u13 : latch port map (
    clk => p1,
    d => p17,
    q => t6,
    qb => p11);

  p14_tp3 <= t6;

  u14 : g2 port map (
    a => t6,
    b => f,
    y => p12);



end gates;

