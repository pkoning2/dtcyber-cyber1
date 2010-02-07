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
-- PA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity paslice is
    port (
      clk : in  logicsig;
      clk2 : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

end paslice;
architecture gates of paslice is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component latch2
    port (
      clk : in  logicsig;
      clk2 : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal tq : logicsig;
  signal tq2 : logicsig;

begin -- gates
  u1 : latch2 port map (
    clk => clk,
    clk2 => clk2,
    d => d,
    q => tq);

  tp <= tq;

  u2 : inv2 port map (
    a => tq,
    y2 => tq2);

  q1 <= tq2;
  q2 <= tq2;


end gates;

use work.sigs.all;

entity pa is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
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
      p10 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pa;
architecture gates of pa is
  component paslice
    port (
      clk : in  logicsig;
      clk2 : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

  end component;


begin -- gates
  u5 : paslice port map (
    clk => p9,
    clk2 => p7,
    d => p8,
    q1 => p1,
    q2 => p3,
    tp => tp1);


  u6 : paslice port map (
    clk => p9,
    clk2 => p7,
    d => p5,
    q1 => p2,
    q2 => p6,
    tp => tp2);


  u7 : paslice port map (
    clk => p9,
    clk2 => p7,
    d => p12,
    q1 => p14,
    q2 => p10,
    tp => tp3);


  u8 : paslice port map (
    clk => p20,
    clk2 => p22,
    d => p21,
    q1 => p19,
    q2 => p17,
    tp => tp4);


  u9 : paslice port map (
    clk => p20,
    clk2 => p22,
    d => p24,
    q1 => p27,
    q2 => p25,
    tp => tp5);


  u10 : paslice port map (
    clk => p20,
    clk2 => p22,
    d => p23,
    q1 => p28,
    q2 => p26,
    tp => tp6);



end gates;

