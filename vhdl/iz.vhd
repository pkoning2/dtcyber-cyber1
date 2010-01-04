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
-- IZ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity izslice is
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      y : out coaxsig;
      y2 : out coaxsig);

end izslice;
architecture gates of izslice is
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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : coaxsig;

begin -- gates
  u1 : g4 port map (
    a => i1,
    b => i2,
    c => i3,
    d => i4,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => a,
    y => t2);

  tp <= t2;

  u3 : cxdriver port map (
    a => t2,
    y => t3);

  y <= t3;
  y2 <= t3;


end gates;

use work.sigs.all;

entity iz is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out coaxsig;
      p3 : out coaxsig;
      p5 : out coaxsig;
      p7 : out coaxsig;
      p22 : out coaxsig;
      p24 : out coaxsig;
      p26 : out coaxsig;
      p28 : out coaxsig);

end iz;
architecture gates of iz is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component izslice
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      y : out coaxsig;
      y2 : out coaxsig);

  end component;

  signal a : logicsig;

begin -- gates
  u1 : inv port map (
    a => p17,
    y => a);


  u2 : izslice port map (
    a => a,
    i1 => p4,
    i2 => p6,
    i3 => p8,
    i4 => p10,
    tp => tp1,
    y => p7,
    y2 => p5);


  u3 : izslice port map (
    a => a,
    i1 => p9,
    i2 => p11,
    i3 => p13,
    i4 => p15,
    tp => tp2,
    y => p3,
    y2 => p1);


  u4 : izslice port map (
    a => a,
    i1 => p14,
    i2 => p16,
    i3 => p18,
    i4 => p20,
    tp => tp5,
    y => p26,
    y2 => p28);


  u5 : izslice port map (
    a => a,
    i1 => p25,
    i2 => p23,
    i3 => p21,
    i4 => p19,
    tp => tp6,
    y => p22,
    y2 => p24);



end gates;

