-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- QG module, rev C
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
      p2_p4_p8_p22_p26_p28 : out logicsig;
      p7_p9_p10_p11_p12_p13 : out logicsig;
      p15_p16_p17_p18_p20_p27 : out logicsig);

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
      qb : out logicsig;
      qs : out logicsig);

  end component;

  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal t1 : logicsig;
  signal t3 : logicsig;

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
    y2 => p7_p9_p10_p11_p12_p13);


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
    y2 => p15_p16_p17_p18_p20_p27);


  u7 : g2 port map (
    a => c,
    b => d,
    y => e);

  tp6 <= e;

  u8 : inv2 port map (
    a => e,
    y => p2_p4_p8_p22_p26_p28);


  u9 : inv port map (
    a => p5,
    y => tp2);



end gates;

