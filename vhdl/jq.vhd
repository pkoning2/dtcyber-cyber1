-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- JQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jqslice is
    port (
      a : in  logicsig;
      i : in  logicsig;
      tp : out logicsig;
      y : out coaxsig);

end jqslice;
architecture gates of jqslice is
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

  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i,
    b => a,
    y => t1);

  tp <= t1;

  u2 : cxdriver port map (
    a => t1,
    y => y);



end gates;

use work.sigs.all;

entity jq is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2_tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out coaxsig;
      p6 : out coaxsig;
      p8 : out coaxsig;
      p10 : out coaxsig;
      p12 : out coaxsig;
      p19 : out coaxsig;
      p21 : out coaxsig;
      p23 : out coaxsig;
      p25 : out coaxsig;
      p27 : out coaxsig);

end jq;
architecture gates of jq is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component jqslice
    port (
      a : in  logicsig;
      i : in  logicsig;
      tp : out logicsig;
      y : out coaxsig);

  end component;

  signal a : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p14,
    b => p17,
    y2 => a);

  tp2_tp5 <= a;

  u2 : jqslice port map (
    a => a,
    i => p5,
    tp => tp1,
    y => p12);


  u3 : jqslice port map (
    a => a,
    i => p7,
    y => p10);


  u4 : jqslice port map (
    a => a,
    i => p9,
    y => p8);


  u5 : jqslice port map (
    a => a,
    i => p11,
    y => p6);


  u6 : jqslice port map (
    a => a,
    i => p13,
    y => p4);


  u7 : jqslice port map (
    a => a,
    i => p26,
    tp => tp6,
    y => p19);


  u8 : jqslice port map (
    a => a,
    i => p24,
    y => p21);


  u9 : jqslice port map (
    a => a,
    i => p22,
    y => p23);


  u10 : jqslice port map (
    a => a,
    i => p20,
    y => p25);


  u11 : jqslice port map (
    a => a,
    i => p18,
    y => p27);



end gates;

