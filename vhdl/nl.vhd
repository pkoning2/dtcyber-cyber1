-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- NL module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nlslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end nlslice;
architecture gates of nlslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t1);


  u2 : rsflop port map (
    r => d,
    s => t1,
    q => t2);


  u3 : g2 port map (
    a => t2,
    b => e,
    y => y2);


  u4 : inv port map (
    a => t2,
    y => y1);



end gates;

use work.sigs.all;

entity nlslice2 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end nlslice2;
architecture gates of nlslice2 is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);

  y <= t1;

  u2 : rsflop port map (
    r => c,
    s => t1,
    q => t2);


  u3 : g2 port map (
    a => t2,
    b => d,
    y => y2);


  u4 : inv port map (
    a => t2,
    y => y1);



end gates;

use work.sigs.all;

entity nl is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p3_tp1 : out logicsig;
      p5 : out logicsig;
      p7_tp2 : out logicsig;
      p9 : out logicsig;
      p11_tp3 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22_tp4 : out logicsig;
      p25_tp5 : out logicsig;
      p26_tp6 : out logicsig;
      p27 : out logicsig);

end nl;
architecture gates of nl is
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

  component nlslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;

  component nlslice2
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p13,
    y => b);


  u2 : inv port map (
    a => p14,
    y => a);


  u3 : inv port map (
    a => p16,
    y => c);

  p18 <= c;

  u4 : nlslice port map (
    a => p2,
    b => p4,
    c => a,
    d => b,
    e => c,
    y1 => p3_tp1,
    y2 => p1);


  u5 : nlslice port map (
    a => p6,
    b => p8,
    c => a,
    d => b,
    e => c,
    y1 => p7_tp2,
    y2 => p5);


  u6 : nlslice port map (
    a => p10,
    b => p12,
    c => a,
    d => b,
    e => c,
    y1 => p11_tp3,
    y2 => p9);


  u7 : nlslice2 port map (
    a => p23,
    b => a,
    c => b,
    d => c,
    y1 => p22_tp4,
    y2 => p20);


  u8 : nlslice2 port map (
    a => p24,
    b => a,
    c => b,
    d => c,
    y1 => p25_tp5,
    y2 => p21);


  u9 : nlslice2 port map (
    a => p28,
    b => a,
    c => b,
    d => c,
    y => t1,
    y1 => p26_tp6,
    y2 => p27);


  u10 : g2 port map (
    a => t1,
    b => p17,
    y => p15);



end gates;

