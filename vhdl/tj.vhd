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
-- TJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tjslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      tp : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end tjslice;
architecture gates of tjslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => a,
    y => t1,
    y2 => t2);

  tp <= t2;

  u2 : g2 port map (
    a => t1,
    b => b,
    y => t3);


  u3 : g2 port map (
    a => t2,
    b => c,
    y => t4);


  u4 : g2 port map (
    a => t3,
    b => t4,
    y => t5);

  y1 <= t5;
  y2 <= t5;


end gates;

use work.sigs.all;

entity tj is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p27 : out logicsig);

end tj;
architecture gates of tj is
  component tjslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      tp : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : tjslice port map (
    a => p10,
    b => p9,
    c => p8,
    tp => tp1,
    y1 => p12,
    y2 => p14);


  u2 : tjslice port map (
    a => p7,
    b => p6,
    c => p5,
    tp => tp2,
    y1 => p4,
    y2 => p11);


  u3 : tjslice port map (
    a => p24,
    b => p25,
    c => p26,
    tp => tp5,
    y1 => p27,
    y2 => p20);


  u4 : tjslice port map (
    a => p21,
    b => p22,
    c => p23,
    tp => tp6,
    y1 => p19,
    y2 => p17);



end gates;

