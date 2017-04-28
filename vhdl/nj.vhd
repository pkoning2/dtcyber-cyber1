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
-- NJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity njslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

end njslice;
architecture gates of njslice is
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
    r => e,
    s => t1,
    q => t2);


  u3 : g2 port map (
    a => d,
    b => t2,
    y => q1);


  u4 : inv port map (
    a => t2,
    y => q2);



end gates;

use work.sigs.all;

entity njslice2 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

end njslice2;
architecture gates of njslice2 is
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


  u2 : rsflop port map (
    r => d,
    s => t1,
    q => t2);


  u3 : g2 port map (
    a => c,
    b => t2,
    y => q1);


  u4 : inv port map (
    a => t2,
    y => q2);



end gates;

use work.sigs.all;

entity nj is
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

end nj;
architecture gates of nj is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component njslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

  end component;

  component njslice2
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv port map (
    a => p17,
    y => p15);


  u2 : njslice port map (
    a => p2,
    b => p4,
    c => a,
    d => c,
    e => b,
    q1 => p1,
    q2 => p3_tp1);


  u3 : inv port map (
    a => p16,
    y => c);

  p18 <= c;

  u4 : njslice port map (
    a => p6,
    b => p8,
    c => a,
    d => c,
    e => b,
    q1 => p5,
    q2 => p7_tp2);


  u5 : njslice port map (
    a => p10,
    b => p12,
    c => a,
    d => c,
    e => b,
    q1 => p9,
    q2 => p11_tp3);


  u6 : inv port map (
    a => p14,
    y => a);


  u7 : inv port map (
    a => p13,
    y => b);


  u8 : njslice2 port map (
    a => p23,
    b => a,
    c => c,
    d => b,
    q1 => p20,
    q2 => p22_tp4);


  u9 : njslice2 port map (
    a => p24,
    b => a,
    c => c,
    d => b,
    q1 => p21,
    q2 => p25_tp5);


  u10 : njslice2 port map (
    a => p28,
    b => a,
    c => c,
    d => b,
    q1 => p27,
    q2 => p26_tp6);



end gates;

