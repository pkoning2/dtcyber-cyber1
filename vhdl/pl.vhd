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
-- PL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity plslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      tp : out logicsig;
      y : out coaxsig);

end plslice;
architecture gates of plslice is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t,
    y2 => tp);


  u2 : cxdriver port map (
    a => t,
    y => y);



end gates;

use work.sigs.all;

entity pl is
    port (
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out coaxsig;
      p6 : out coaxsig;
      p8 : out coaxsig;
      p23 : out coaxsig;
      p25 : out coaxsig;
      p27 : out coaxsig);

end pl;
architecture gates of pl is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component plslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      tp : out logicsig;
      y : out coaxsig);

  end component;

  signal a : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p21,
    b => p10,
    y2 => a);


  u2 : plslice port map (
    a => p9,
    b => a,
    tp => tp1,
    y => p8);


  u3 : plslice port map (
    a => p11,
    b => a,
    tp => tp2,
    y => p6);


  u4 : plslice port map (
    a => p13,
    b => a,
    tp => tp3,
    y => p4);


  u5 : plslice port map (
    a => p18,
    b => a,
    tp => tp4,
    y => p27);


  u6 : plslice port map (
    a => p20,
    b => a,
    tp => tp5,
    y => p25);


  u7 : plslice port map (
    a => p22,
    b => a,
    tp => tp6,
    y => p23);



end gates;

