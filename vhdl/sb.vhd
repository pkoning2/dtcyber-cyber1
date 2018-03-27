-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2018 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- SB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity sb is
    port (
      p11 : in  coaxsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  coaxsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  coaxsig;
      p18 : in  coaxsig;
      p19 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3_p4_P5_p6 : out logicsig;
      p10_p9_p8_p7 : out logicsig;
      p23_p22_p21_p20 : out logicsig;
      p24_p25_p26_p27 : out logicsig);

end sb;
architecture gates of sb is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

  end component;

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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p11,
    y => t1);


  u2 : latchs port map (
    clk => p15,
    d => p12,
    s => t1,
    q => t2);

  tp1 <= t2;

  u3 : inv2 port map (
    a => t2,
    y2 => p3_p4_P5_p6);


  u4 : cxreceiver port map (
    a => p14,
    y => t3);


  u5 : latchs port map (
    clk => p15,
    d => p13,
    s => t3,
    q => t4);

  tp2 <= t4;

  u6 : inv2 port map (
    a => t4,
    y2 => p10_p9_p8_p7);


  u7 : cxreceiver port map (
    a => p17,
    y => t5);


  u8 : latchs port map (
    clk => p15,
    d => p16,
    s => t5,
    q => t6);

  tp5 <= t6;

  u9 : inv2 port map (
    a => t6,
    y2 => p24_p25_p26_p27);


  u10 : cxreceiver port map (
    a => p18,
    y => t7);


  u11 : latchs port map (
    clk => p15,
    d => p19,
    s => t7,
    q => t8);

  tp6 <= t8;

  u12 : inv2 port map (
    a => t8,
    y2 => p23_p22_p21_p20);



end gates;

