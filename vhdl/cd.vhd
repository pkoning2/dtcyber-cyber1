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
-- CD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cdslice is
    port (
      f : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      tp : out logicsig;
      a : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end cdslice;
architecture gates of cdslice is
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
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal ta : logicsig;

begin -- gates
  u1 : inv port map (
    a => i1,
    y => ta);

  a <= ta;

  u2 : inv port map (
    a => ta,
    y => t1);


  u3 : g2 port map (
    a => ta,
    b => i2,
    y => t2);


  u4 : g2 port map (
    a => i3,
    b => t1,
    y => t3);


  u5 : g2 port map (
    a => t2,
    b => t3,
    y => t4);

  tp <= t4;
  y1 <= t4;
  y2 <= t4;

  u6 : g2 port map (
    a => t4,
    b => f,
    y => y3);



end gates;

use work.sigs.all;

entity cd is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end cd;
architecture gates of cd is
  component cdslice
    port (
      f : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      tp : out logicsig;
      a : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p9,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => p20,
    y => f);


  u3 : inv2 port map (
    a => p12,
    y2 => p6);


  u4 : cdslice port map (
    f => f,
    i1 => p14,
    i2 => p11,
    i3 => p12,
    a => a,
    tp => tp1,
    y1 => p10,
    y2 => p7,
    y3 => p5);


  u5 : cdslice port map (
    f => f,
    i1 => p8,
    i2 => p3,
    i3 => p4,
    a => c,
    tp => tp2,
    y1 => p2,
    y2 => p1,
    y3 => p13);


  u6 : cdslice port map (
    f => f,
    i1 => p23,
    i2 => p26,
    i3 => p25,
    a => d,
    tp => tp5,
    y1 => p27,
    y2 => p28,
    y3 => p18);


  u7 : cdslice port map (
    f => f,
    i1 => p17,
    i2 => p22,
    i3 => p19,
    a => b,
    tp => tp6,
    y1 => p21,
    y2 => p24,
    y3 => p15);


  u8 : g4 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    y => tp4,
    y2 => t2);

  p16 <= t2;
  tp3 <= t2;


end gates;

