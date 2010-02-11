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
-- JP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jp is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p11_p21 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end jp;
architecture gates of jp is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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

  signal h : logicsig;
  signal k : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : latch22 port map (
    clk => p18,
    clk2 => p16,
    d => p24,
    d2 => p23,
    q => t1,
    qb => t2);

  tp5 <= t1;

  u2 : inv port map (
    a => t1,
    y => h);


  u3 : inv port map (
    a => t2,
    y => p26);


  u4 : g2 port map (
    a => p23,
    b => p16,
    y => p25);


  u5 : inv port map (
    a => p3,
    y => p1);


  u6 : latch port map (
    clk => p9,
    d => p3,
    q => t3);

  tp1 <= t3;

  u7 : g2 port map (
    a => t3,
    b => k,
    y => t4);


  u8 : g2 port map (
    a => p2,
    b => p13,
    y => t5);


  u9 : g4 port map (
    a => t1,
    b => t4,
    c => t5,
    d => p27,
    y => t6,
    y2 => p22);

  tp3 <= t6;

  u10 : latch port map (
    clk => p9,
    d => p6,
    q => t7);

  tp2 <= t7;

  u11 : inv port map (
    a => t7,
    y => k);


  u12 : g4 port map (
    a => p27,
    b => p13,
    c => k,
    d => h,
    y => t8);

  tp6 <= t8;

  u13 : g2 port map (
    a => t6,
    b => t8,
    y => p11_p21);



end gates;

