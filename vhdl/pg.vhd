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
-- PG module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pgslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q : out logicsig);

end pgslice;
architecture gates of pgslice is
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal ti : logicsig;

begin -- gates
  u1 : g3 port map (
    a => e,
    b => i1,
    c => b,
    y => t1);


  u2 : g3 port map (
    a => b,
    b => i2,
    c => d,
    y => t2);


  u3 : rs2flop port map (
    r => a,
    s => t1,
    s2 => t2,
    q => ti);

  tp <= ti;

  u4 : inv port map (
    a => ti,
    y => q);



end gates;

use work.sigs.all;

entity pg is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p20 : out logicsig);

end pg;
architecture gates of pg is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component pgslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal d : logicsig;
  signal e : logicsig;

begin -- gates
  u1 : inv port map (
    a => p10,
    y => a);


  u2 : inv2 port map (
    a => p8,
    y2 => b);


  u4 : g2 port map (
    a => p9,
    b => p21,
    y => d);


  u5 : g2 port map (
    a => p7,
    b => p23,
    y => e);


  u6 : pgslice port map (
    a => a,
    b => b,
    d => d,
    e => e,
    i1 => p5,
    i2 => p3,
    q => p13,
    tp => tp1);


  u7 : pgslice port map (
    a => a,
    b => b,
    d => d,
    e => e,
    i1 => p6,
    i2 => p4,
    q => p14,
    tp => tp2);


  u8 : pgslice port map (
    a => a,
    b => b,
    d => d,
    e => e,
    i1 => p25,
    i2 => p27,
    q => p17,
    tp => tp5);


  u9 : pgslice port map (
    a => a,
    b => b,
    d => d,
    e => e,
    i1 => p26,
    i2 => p28,
    q => p20,
    tp => tp6);



end gates;

