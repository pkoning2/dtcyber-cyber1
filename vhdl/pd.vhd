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
-- PD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pdslice is
    port (
      clk : in  logicsig;
      i : in  logicsig;
      tp : out logicsig;
      q : out logicsig;
      qb : out logicsig);

end pdslice;
architecture gates of pdslice is
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

  signal ti : logicsig;

begin -- gates
  u1 : latch port map (
    clk => clk,
    d => i,
    q => ti);

  tp <= ti;

  u2 : inv2 port map (
    a => ti,
    y => qb,
    y2 => q);



end gates;

use work.sigs.all;

entity pd is
    port (
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pd;
architecture gates of pd is
  component pdslice
    port (
      clk : in  logicsig;
      i : in  logicsig;
      tp : out logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal q1 : logicsig;
  signal q2 : logicsig;
  signal q3 : logicsig;
  signal q4 : logicsig;
  signal qb1 : logicsig;
  signal qb2 : logicsig;
  signal qb3 : logicsig;
  signal qb4 : logicsig;

begin -- gates
  u1 : pdslice port map (
    clk => p16,
    i => p14,
    q => q1,
    qb => qb1,
    tp => tp1);

  p4 <= qb1;
  p6 <= qb1;
  p7 <= q1;
  p9 <= q1;
  p11 <= q1;

  u2 : pdslice port map (
    clk => p16,
    i => p13,
    q => q2,
    qb => qb2,
    tp => tp2);

  p3 <= qb2;
  p5 <= qb2;
  p8 <= q2;
  p10 <= q2;
  p12 <= q2;

  u3 : pdslice port map (
    clk => p16,
    i => p18,
    q => q3,
    qb => qb3,
    tp => tp5);

  p19 <= q3;
  p20 <= q3;
  p23 <= qb3;
  p25 <= q3;
  p28 <= qb3;

  u4 : pdslice port map (
    clk => p16,
    i => p17,
    q => q4,
    qb => qb4,
    tp => tp6);

  p21 <= qb4;
  p22 <= q4;
  p24 <= q4;
  p26 <= q4;
  p27 <= qb4;


end gates;

