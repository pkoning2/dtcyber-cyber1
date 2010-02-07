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
-- JH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jhslice is
    port (
      c : in  logicsig;
      i : in  logicsig;
      tp : out logicsig;
      y1 : out coaxsig;
      y2 : out coaxsig;
      y3 : out coaxsig;
      y4 : out coaxsig);

end jhslice;
architecture gates of jhslice is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : coaxsig;

begin -- gates
  u1 : inv port map (
    a => i,
    y => t1);


  u2 : g2 port map (
    a => c,
    b => t1,
    y => t2);

  tp <= t2;

  u3 : cxdriver port map (
    a => t2,
    y => t3);

  y1 <= t3;
  y2 <= t3;
  y3 <= t3;
  y4 <= t3;


end gates;

use work.sigs.all;

entity jh is
    port (
      p4 : in  logicsig;
      p12 : in  logicsig;
      p19 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      p3 : out coaxsig;
      p5 : out coaxsig;
      p7 : out coaxsig;
      p9 : out coaxsig;
      p11 : out coaxsig;
      p13 : out coaxsig;
      p18 : out coaxsig;
      p20 : out coaxsig;
      p22 : out coaxsig;
      p24 : out coaxsig;
      p26 : out coaxsig;
      p28 : out coaxsig);

end jh;
architecture gates of jh is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component jhslice
    port (
      c : in  logicsig;
      i : in  logicsig;
      tp : out logicsig;
      y1 : out coaxsig;
      y2 : out coaxsig;
      y3 : out coaxsig;
      y4 : out coaxsig);

  end component;

  signal c : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p19,
    y2 => c);


  u2 : jhslice port map (
    c => c,
    i => p12,
    tp => tp1,
    y1 => p3,
    y2 => p5,
    y3 => p26,
    y4 => p28);


  u3 : jhslice port map (
    c => c,
    i => p4,
    tp => tp2,
    y1 => p9,
    y2 => p7,
    y3 => p13,
    y4 => p11);


  u4 : jhslice port map (
    c => c,
    i => p27,
    tp => tp4,
    y1 => p18,
    y2 => p20,
    y3 => p22,
    y4 => p24);



end gates;

