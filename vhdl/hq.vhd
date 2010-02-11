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
-- HQ module -- clock receiver
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hq is
    port (
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  coaxsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3_p5_p7 : out logicsig;
      p2_p4_p6_p8 : out logicsig;
      p14_tp1 : out logicsig;
      p21_p23_p25_p27 : out logicsig;
      p22_p24_p26_p28 : out logicsig);

end hq;
architecture gates of hq is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

  end component;

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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p10,
    y => t1);

  tp4 <= t1;

  u2 : cxreceiver port map (
    a => p12,
    y => t2);


  u3 : rsflop port map (
    r => t1,
    s => t2,
    q => p14_tp1);


  u4 : inv port map (
    a => p16,
    y => t4);


  u5 : g2 port map (
    a => t4,
    b => p11,
    y => tp2,
    y2 => p2_p4_p6_p8);


  u6 : inv port map (
    a => p13,
    y => t6);


  u7 : g2 port map (
    a => t6,
    b => p18,
    y => tp3,
    y2 => p1_p3_p5_p7);


  u8 : inv port map (
    a => p15,
    y => t8);


  u9 : g2 port map (
    a => t8,
    b => p17,
    y => tp5,
    y2 => p22_p24_p26_p28);


  u10 : inv port map (
    a => p19,
    y => t10);


  u11 : g2 port map (
    a => t10,
    b => p20,
    y => tp6,
    y2 => p21_p23_p25_p27);



end gates;

