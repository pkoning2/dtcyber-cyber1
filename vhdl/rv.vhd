-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- RV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rvslice is
    port (
      clk : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig := '1';
      tp : out logicsig;
      q1_q3 : out logicsig;
      q2 : out logicsig);

end rvslice;
architecture gates of rvslice is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component latchs
    port (
      clk : in  logicsig;
      d : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t2 : logicsig;

begin -- gates
  u2 : latchs port map (
    clk => clk,
    d => i,
    s => i2,
    q => t2);

  tp <= t2;

  u3 : inv2 port map (
    a => t2,
    y => q2,
    y2 => q1_q3);



end gates;

use work.sigs.all;

entity rv is
    port (
      p6 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2_p11 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5_p19 : out logicsig;
      p7_p9 : out logicsig;
      p8_p10 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25_p27 : out logicsig);

end rv;
architecture gates of rv is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rvslice
    port (
      clk : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig := '1';
      tp : out logicsig;
      q1_q3 : out logicsig;
      q2 : out logicsig);

  end component;

  signal t2 : logicsig;

begin -- gates
  u3 : rvslice port map (
    clk => p16,
    i => p13,
    i2 => p6,
    q1_q3 => p8_p10,
    q2 => p3,
    tp => tp1);


  u4 : rvslice port map (
    clk => p16,
    i => p18,
    q1_q3 => p5_p19,
    q2 => p23,
    tp => tp2);


  u5 : rvslice port map (
    clk => p16,
    i => p14,
    q1_q3 => p7_p9,
    q2 => p4,
    tp => tp3);


  u6 : rvslice port map (
    clk => p28,
    i => p26,
    q1_q3 => p25_p27,
    q2 => p20,
    tp => tp4);


  u7 : rvslice port map (
    clk => p28,
    i => p15,
    q1_q3 => p2_p11,
    q2 => p1,
    tp => tp5);


  u8 : rvslice port map (
    clk => p28,
    i => p17,
    q1_q3 => p24,
    q2 => t2,
    tp => tp6);

  p21 <= t2;

  u9 : g2 port map (
    a => p12,
    b => t2,
    y => p22);



end gates;

