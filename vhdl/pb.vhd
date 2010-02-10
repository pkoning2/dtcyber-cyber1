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
-- PB module rev D -- barrel latches
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pbslice is
    port (
      c1 : in  logicsig;
      c2 : in  logicsig;
      c3 : in  logicsig;
      i : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      q : out logicsig);

end pbslice;
architecture gates of pbslice is
  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal tq1 : logicsig;
  signal tq2 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => c1,
    d => i,
    q => tq1);

  tp1 <= tq1;

  u2 : latch port map (
    clk => c2,
    d => tq1,
    q => tq2);

  tp2 <= tq2;

  u3 : latch port map (
    clk => c3,
    d => tq2,
    q => q);



end gates;

use work.sigs.all;

entity pb is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p22 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p9 : out logicsig);

end pb;
architecture gates of pb is
  component pbslice
    port (
      c1 : in  logicsig;
      c2 : in  logicsig;
      c3 : in  logicsig;
      i : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      q : out logicsig);

  end component;


begin -- gates
  u1 : pbslice port map (
    c1 => p5,
    c2 => p22,
    c3 => p27,
    i => p7,
    q => p9,
    tp1 => tp1,
    tp2 => tp4);


  u2 : pbslice port map (
    c1 => p5,
    c2 => p22,
    c3 => p27,
    i => p6,
    q => p3,
    tp1 => tp2,
    tp2 => tp5);


  u3 : pbslice port map (
    c1 => p5,
    c2 => p22,
    c3 => p27,
    i => p10,
    q => p1,
    tp1 => tp3,
    tp2 => tp6);



end gates;

