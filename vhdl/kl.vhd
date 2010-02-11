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
-- KL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kl is
    port (
      p2 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p27 : in  logicsig;
      p3_p4_p5_p6 : out logicsig;
      p7_p8_p9_p10 : out logicsig;
      p11_tp5 : out logicsig;
      p12_tp6 : out logicsig;
      p17_tp2 : out logicsig;
      p19_p21_p22_p24 : out logicsig;
      p20_tp1 : out logicsig;
      p23_p25_p26_p28 : out logicsig);

end kl;
architecture gates of kl is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t3 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p2,
    b => p14,
    y => t1);

  p17_tp2 <= t1;

  u2 : inv2 port map (
    a => t1,
    y2 => p3_p4_p5_p6);


  u3 : g2 port map (
    a => p14,
    b => p16,
    y => t3);

  p20_tp1 <= t3;

  u4 : inv2 port map (
    a => t3,
    y2 => p7_p8_p9_p10);


  u5 : g2 port map (
    a => p27,
    b => p15,
    y => t5);

  p11_tp5 <= t5;

  u6 : inv2 port map (
    a => t5,
    y2 => p23_p25_p26_p28);


  u7 : g2 port map (
    a => p15,
    b => p13,
    y => t7);

  p12_tp6 <= t7;

  u8 : inv2 port map (
    a => t7,
    y2 => p19_p21_p22_p24);



end gates;

