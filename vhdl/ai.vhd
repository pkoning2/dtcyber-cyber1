-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- AI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity aislice is
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      y1 : out coaxsig;
      y2 : out coaxsig;
      y3 : out coaxsig);

end aislice;
architecture gates of aislice is
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
  signal t2 : logicsig;
  signal t3 : coaxsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => i2,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => a,
    y => t2);

  tp <= t2;

  u3 : cxdriver port map (
    a => t2,
    y => t3);

  y1 <= t3;
  y2 <= t3;
  y3 <= t3;


end gates;

use work.sigs.all;

entity ai is
    port (
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p3 : out coaxsig;
      p4 : out coaxsig;
      p5 : out coaxsig;
      p6 : out coaxsig;
      p7 : out coaxsig;
      p8 : out coaxsig;
      p23 : out coaxsig;
      p24 : out coaxsig;
      p25 : out coaxsig;
      p26 : out coaxsig;
      p27 : out coaxsig;
      p28 : out coaxsig);

end ai;
architecture gates of ai is
  component aislice
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      y1 : out coaxsig;
      y2 : out coaxsig;
      y3 : out coaxsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;

begin -- gates
  u1 : inv port map (
    a => p17,
    y => a);


  u2 : aislice port map (
    a => a,
    i1 => p11,
    i2 => p9,
    tp => tp2,
    y1 => p7,
    y2 => p3,
    y3 => p5);


  u3 : aislice port map (
    a => a,
    i1 => p21,
    i2 => p19,
    tp => tp4,
    y1 => p23,
    y2 => p25,
    y3 => p27);


  u4 : aislice port map (
    a => a,
    i1 => p10,
    i2 => p12,
    tp => tp3,
    y1 => p8,
    y2 => p4,
    y3 => p6);


  u5 : aislice port map (
    a => a,
    i1 => p20,
    i2 => p22,
    tp => tp5,
    y1 => p24,
    y2 => p28,
    y3 => p26);



end gates;

