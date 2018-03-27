-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2018 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- NZ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nzslice is
    port (
      b : in  logicsig;
      d : in  logicsig;
      f : in  logicsig;
      i : in  logicsig;
      y : out logicsig);

end nzslice;
architecture gates of nzslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i,
    b => b,
    y => t1);


  u2 : rsflop port map (
    r => f,
    s => t1,
    q => t2);


  u3 : g2 port map (
    a => t2,
    b => d,
    y => y);



end gates;

use work.sigs.all;

entity nz is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end nz;
architecture gates of nz is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component nzslice
    port (
      b : in  logicsig;
      d : in  logicsig;
      f : in  logicsig;
      i : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal d : logicsig;
  signal f : logicsig;

begin -- gates
  u1 : inv port map (
    a => p18,
    y => a);


  u2 : inv port map (
    a => p12,
    y => b);


  u3 : inv port map (
    a => p10,
    y => f);


  u4 : inv port map (
    a => p19,
    y => d);


  u5 : nzslice port map (
    b => b,
    d => d,
    f => f,
    i => p2,
    y => p1);


  u6 : nzslice port map (
    b => b,
    d => d,
    f => f,
    i => p3,
    y => p5);


  u7 : nzslice port map (
    b => b,
    d => d,
    f => f,
    i => p4,
    y => p6);


  u8 : nzslice port map (
    b => b,
    d => d,
    f => f,
    i => p7,
    y => p8);


  u9 : nzslice port map (
    b => a,
    d => p23,
    f => f,
    i => p23,
    y => p21);


  u10 : nzslice port map (
    b => a,
    d => d,
    f => f,
    i => p22,
    y => p25);


  u11 : nzslice port map (
    b => a,
    d => d,
    f => f,
    i => p26,
    y => p24);


  u12 : nzslice port map (
    b => a,
    d => d,
    f => f,
    i => p27,
    y => p28);



end gates;

