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
-- QK module, rev D -- incrementer
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qkslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      k : in  logicsig;
      k2 : in  logicsig := '1';
      k3 : in  logicsig := '1';
      tp : out logicsig;
      y : out logicsig);

end qkslice;
architecture gates of qkslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => k,
    b => k2,
    c => k3,
    y => t1,
    y2 => t2);


  u2 : g2 port map (
    a => b,
    b => t2,
    y => t3);


  u3 : g2 port map (
    a => t1,
    b => a,
    y => t4);


  u4 : g2 port map (
    a => t3,
    b => t4,
    y => t5);

  tp <= t5;
  y <= t5;


end gates;

use work.sigs.all;

entity qk is
    port (
      p13 : in  logicsig;
      p14 : in  logicsig;
      p18 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p20 : out logicsig;
      p24 : out logicsig);

end qk;
architecture gates of qk is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
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

  component qkslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      k : in  logicsig;
      k2 : in  logicsig := '1';
      k3 : in  logicsig := '1';
      tp : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal k : logicsig;
  signal t : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p13,
    y2 => a);

  tp3 <= a;

  u2 : inv port map (
    a => a,
    y => b);


  u3 : inv2 port map (
    a => p14,
    y2 => c);

  tp2 <= c;

  u4 : inv port map (
    a => c,
    y => d);


  u5 : inv2 port map (
    a => p18,
    y2 => e);

  tp4 <= e;

  u6 : inv port map (
    a => e,
    y => f);


  u7 : g4 port map (
    a => p26,
    b => p25,
    c => p28,
    d => p27,
    y2 => k);


  u8 : qkslice port map (
    a => a,
    b => b,
    k => k,
    tp => tp1,
    y => p9);


  u9 : qkslice port map (
    a => c,
    b => d,
    k => k,
    k2 => a,
    tp => tp6,
    y => p24);


  u10 : qkslice port map (
    a => e,
    b => f,
    k => k,
    k2 => a,
    k3 => c,
    tp => tp5,
    y => p20);


  u11 : g3 port map (
    a => a,
    b => c,
    c => e,
    y2 => t);

  p3 <= t;
  p5 <= t;
  p7 <= t;


end gates;

