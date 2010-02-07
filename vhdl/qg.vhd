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
-- QG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qg is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end qg;
architecture gates of qg is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p1,
    b => p3,
    y => t1);


  u2 : latch port map (
    clk => p5,
    d => t1,
    q => c);

  tp1 <= c;

  u3 : inv2 port map (
    a => c,
    y2 => t2);

  p7 <= t2;
  p9 <= t2;
  p10 <= t2;
  p11 <= t2;
  p12 <= t2;
  p13 <= t2;

  u4 : g2 port map (
    a => p25,
    b => p23,
    y => t3);


  u5 : latch port map (
    clk => p5,
    d => t3,
    q => d);

  tp5 <= d;

  u6 : inv2 port map (
    a => d,
    y2 => t4);

  p15 <= t4;
  p16 <= t4;
  p17 <= t4;
  p18 <= t4;
  p20 <= t4;
  p27 <= t4;

  u7 : g2 port map (
    a => c,
    b => d,
    y => e);

  tp6 <= e;

  u8 : inv2 port map (
    a => e,
    y => t5);

  p2 <= t5;
  p4 <= t5;
  p8 <= t5;
  p22 <= t5;
  p26 <= t5;
  p28 <= t5;

  u9 : inv port map (
    a => p5,
    y => tp2);



end gates;

