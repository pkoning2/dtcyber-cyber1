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
-- JB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jb is
    port (
      p3 : in  logicsig;
      p6 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end jb;
architecture gates of jb is
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

  component latch22
    port (
      clk : in  logicsig;
      clk2 : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal f : logicsig;
  signal g : logicsig;
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
  u1 : inv port map (
    a => p6,
    y => t1);

  tp1 <= t1;

  u2 : latch22 port map (
    clk => p19,
    clk2 => p15,
    d => t1,
    d2 => p3,
    q => t2);

  p8 <= t2;
  tp2 <= t2;

  u3 : g2 port map (
    a => t2,
    b => f,
    y => t3);


  u4 : latch22 port map (
    clk => p17,
    clk2 => p11,
    d => t1,
    d2 => p3,
    q => t4);

  p12 <= t4;
  tp3 <= t4;

  u5 : g2 port map (
    a => t4,
    b => g,
    y => t5);


  u6 : g2 port map (
    a => t3,
    b => t5,
    y => p10);


  u7 : inv port map (
    a => p25,
    y => t6);


  u8 : latch22 port map (
    clk => p17,
    clk2 => p11,
    d => t1,
    d2 => p26,
    q => t7);

  p24 <= t7;
  tp6 <= t7;

  u9 : g2 port map (
    a => t7,
    b => g,
    y => t8);


  u10 : latch22 port map (
    clk => p19,
    clk2 => p15,
    d => t1,
    d2 => p26,
    q => t9);

  p28 <= t9;
  tp5 <= t9;

  u11 : g2 port map (
    a => t9,
    b => f,
    y => t10);


  u12 : g2 port map (
    a => t8,
    b => t10,
    y => p22);


  u13 : inv2 port map (
    a => p14,
    y => f,
    y2 => g);



end gates;

