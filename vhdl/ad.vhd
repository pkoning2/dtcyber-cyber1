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
-- AD module - 6612 S register
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity adslice is
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      q_tp : out logicsig;
      qb1_qb2 : out logicsig);

end adslice;
architecture gates of adslice is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => s,
    b => e,
    y => t1);


  u2 : rsflop port map (
    r => r,
    s => t1,
    q => q_tp,
    qb => t3);


  u3 : inv2 port map (
    a => t3,
    y2 => qb1_qb2);



end gates;

use work.sigs.all;

entity ad is
    port (
      p4 : in  logicsig;
      p5 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p1_p2 : out logicsig;
      p3_tp2 : out logicsig;
      p6_tp1 : out logicsig;
      p7_p8 : out logicsig;
      p9_p10 : out logicsig;
      p12_tp3 : out logicsig;
      p17_tp4 : out logicsig;
      p19_p20 : out logicsig;
      p21_p22 : out logicsig;
      p23_tp6 : out logicsig;
      p26_tp5 : out logicsig;
      p27_p28 : out logicsig);

end ad;
architecture gates of ad is
  component adslice
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      q_tp : out logicsig;
      qb1_qb2 : out logicsig);

  end component;

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

  signal a : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p16,
    y2 => a);


  u2 : g2 port map (
    a => p15,
    b => p13,
    y2 => c);


  u3 : g2 port map (
    a => p13,
    b => p14,
    y2 => d);


  u4 : adslice port map (
    e => a,
    r => c,
    s => p18,
    q_tp => p17_tp4,
    qb1_qb2 => p19_p20);


  u5 : adslice port map (
    e => a,
    r => d,
    s => p5,
    q_tp => p6_tp1,
    qb1_qb2 => p7_p8);


  u6 : adslice port map (
    e => a,
    r => c,
    s => p25,
    q_tp => p26_tp5,
    qb1_qb2 => p27_p28);


  u7 : adslice port map (
    e => a,
    r => d,
    s => p4,
    q_tp => p3_tp2,
    qb1_qb2 => p1_p2);


  u8 : adslice port map (
    e => a,
    r => c,
    s => p24,
    q_tp => p23_tp6,
    qb1_qb2 => p21_p22);


  u9 : adslice port map (
    e => a,
    r => d,
    s => p11,
    q_tp => p12_tp3,
    qb1_qb2 => p9_p10);



end gates;

