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
-- AD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity adslice is
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      q : out logicsig;
      qb1 : out logicsig;
      qb2 : out logicsig);

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
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => s,
    b => e,
    y => t1);


  u2 : rsflop port map (
    r => r,
    s => t1,
    q => t2,
    qb => t3);

  q <= t2;
  tp <= t2;

  u3 : inv2 port map (
    a => t3,
    y => t4);

  qb1 <= t4;
  qb2 <= t4;


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
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ad;
architecture gates of ad is
  component adslice
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      q : out logicsig;
      qb1 : out logicsig;
      qb2 : out logicsig);

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
    q => p17,
    qb1 => p19,
    qb2 => p20,
    tp => tp4);


  u5 : adslice port map (
    e => a,
    r => d,
    s => p5,
    q => p6,
    qb1 => p8,
    qb2 => p7,
    tp => tp1);


  u6 : adslice port map (
    e => a,
    r => c,
    s => p25,
    q => p26,
    qb1 => p28,
    qb2 => p27,
    tp => tp5);


  u7 : adslice port map (
    e => a,
    r => d,
    s => p4,
    q => p3,
    qb1 => p1,
    qb2 => p2,
    tp => tp2);


  u8 : adslice port map (
    e => a,
    r => c,
    s => p24,
    q => p23,
    qb1 => p21,
    qb2 => p22,
    tp => tp6);


  u9 : adslice port map (
    e => a,
    r => d,
    s => p11,
    q => p12,
    qb1 => p10,
    qb2 => p9,
    tp => tp3);



end gates;

