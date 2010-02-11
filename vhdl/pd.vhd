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
-- PD module rev D
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
      p3_p5 : out logicsig;
      p4_p6 : out logicsig;
      p7_p9_p11 : out logicsig;
      p8_p10_p12 : out logicsig;
      p19_p20_p25 : out logicsig;
      p21_p27 : out logicsig;
      p22_p24_p26 : out logicsig;
      p23_p28 : out logicsig);

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


begin -- gates
  u1 : pdslice port map (
    clk => p16,
    i => p14,
    q => p7_p9_p11,
    qb => p4_p6,
    tp => tp1);


  u2 : pdslice port map (
    clk => p16,
    i => p13,
    q => p8_p10_p12,
    qb => p3_p5,
    tp => tp2);


  u3 : pdslice port map (
    clk => p16,
    i => p18,
    q => p19_p20_p25,
    qb => p23_p28,
    tp => tp5);


  u4 : pdslice port map (
    clk => p16,
    i => p17,
    q => p22_p24_p26,
    qb => p21_p27,
    tp => tp6);



end gates;

