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
-- AA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity aaslice is
    port (
      r1 : in  logicsig;
      r2 : in  logicsig;
      s1 : in  logicsig;
      s2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

end aaslice;
architecture gates of aaslice is
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
    a => s1,
    b => s2,
    y => t1);


  u2 : g2 port map (
    a => r1,
    b => r2,
    y => t2);


  u3 : rsflop port map (
    r => t2,
    s => t1,
    q => t3,
    qb => qb);

  tp <= t3;

  u4 : inv2 port map (
    a => t3,
    y2 => t4);

  q1 <= t4;
  q2 <= t4;


end gates;

use work.sigs.all;

entity aa is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end aa;
architecture gates of aa is
  component aaslice
    port (
      r1 : in  logicsig;
      r2 : in  logicsig;
      s1 : in  logicsig;
      s2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

  end component;


begin -- gates
  u1 : aaslice port map (
    r1 => p6,
    r2 => p8,
    s1 => p5,
    s2 => p7,
    q1 => p2,
    q2 => p4,
    qb => p9,
    tp => tp1);


  u2 : aaslice port map (
    r1 => p11,
    r2 => p13,
    s1 => p12,
    s2 => p14,
    q1 => p1,
    q2 => p3,
    qb => p10,
    tp => tp2);


  u3 : aaslice port map (
    r1 => p16,
    r2 => p18,
    s1 => p15,
    s2 => p17,
    q1 => p26,
    q2 => p28,
    qb => p19,
    tp => tp5);


  u4 : aaslice port map (
    r1 => p21,
    r2 => p23,
    s1 => p22,
    s2 => p24,
    q1 => p25,
    q2 => p27,
    qb => p20,
    tp => tp6);



end gates;

