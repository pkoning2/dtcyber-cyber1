-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- CZ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity czslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      d : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig := '0';
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig);

end czslice;
architecture gates of czslice is
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
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : inv port map (
    a => i,
    y => t1);

  q1 <= t1;

  u2 : g2 port map (
    a => t1,
    b => a,
    y => t2);


  u3 : rsflop port map (
    r => b,
    s => t2,
    q => t3);

  q2 <= t3;
  tp <= t3;

  u4 : g2 port map (
    a => d,
    b => t3,
    y => t4);


  u5 : g2 port map (
    a => t4,
    b => i2,
    y => t5);

  q3 <= t5;
  q4 <= t5;


end gates;

use work.sigs.all;

entity cz is
    port (
      p3 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p26 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end cz;
architecture gates of cz is
  component czslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      d : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig);

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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : czslice port map (
    a => a,
    b => b,
    i => p26,
    i2 => t1,
    q1 => p28,
    q3 => p25,
    q4 => p27,
    tp => tp5);


  u2 : czslice port map (
    a => a,
    b => b,
    d => d,
    i => p22,
    i2 => t2,
    q1 => p23,
    q2 => t3,
    q3 => p24,
    q4 => p11,
    tp => tp6);


  u3 : g2 port map (
    a => t3,
    b => c,
    y => t1);


  u4 : czslice port map (
    a => a,
    b => b,
    d => d,
    i => p20,
    i2 => t4,
    q1 => p21,
    q2 => t5,
    q3 => p8,
    q4 => p9,
    tp => tp3);


  u5 : g2 port map (
    a => t5,
    b => c,
    y => t2);


  u6 : czslice port map (
    a => a,
    b => b,
    d => d,
    i => p3,
    q1 => p1,
    q2 => t6,
    q3 => p5,
    q4 => p7,
    tp => tp2);


  u7 : g2 port map (
    a => t6,
    b => c,
    y => t4);


  u8 : g2 port map (
    a => p19,
    b => a,
    y => t7);


  u9 : rsflop port map (
    r => b,
    s => t7,
    q => t8);


  u10 : inv port map (
    a => t8,
    y => c);


  u11 : inv2 port map (
    a => p18,
    y => b,
    y2 => a);



end gates;

