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
-- PU module, rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity puslice is
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

end puslice;
architecture gates of puslice is
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

  signal tqbi : logicsig;
  signal tqi : logicsig;
  signal tri : logicsig;
  signal tsi : logicsig;

begin -- gates
  u1 : g2 port map (
    a => s,
    b => e,
    y => tsi);


  u2 : g2 port map (
    a => r,
    b => e,
    y => tri);


  u3 : rsflop port map (
    r => tri,
    s => tsi,
    q => tqi,
    qb => tqbi);

  tp <= tqi;

  u4 : inv2 port map (
    a => tqi,
    y => q1,
    y2 => q2);


  u5 : inv2 port map (
    a => tqbi,
    y2 => qb);



end gates;

use work.sigs.all;

entity pu is
    port (
      p10 : in  logicsig;
      p11 : in  logicsig;
      p14 : inout misc;
      p15 : inout misc;
      p16 : inout misc;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pu;
architecture gates of pu is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component puslice
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p17,
    y2 => a);

  tp5 <= a;

  u2 : puslice port map (
    e => a,
    r => p11,
    s => p10,
    q1 => p9,
    q2 => p8,
    qb => t1,
    tp => tp1);

  p1 <= t1;
  p3 <= t1;
  p4 <= t1;
  p6 <= t1;
  p7 <= t1;

  u3 : puslice port map (
    e => a,
    r => p18,
    s => p19,
    q1 => p22,
    q2 => p21,
    qb => t2,
    tp => tp6);

  p24 <= t2;
  p25 <= t2;
  p26 <= t2;
  p27 <= t2;
  p28 <= t2;


end gates;

