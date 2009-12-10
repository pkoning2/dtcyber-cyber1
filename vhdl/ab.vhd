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
-- AB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity abslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

end abslice;
architecture gates of abslice is
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
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);


  u2 : inv port map (
    a => a,
    y => t2);


  u3 : inv port map (
    a => b,
    y => t3);


  u4 : g2 port map (
    a => t2,
    b => t3,
    y => t4);


  u5 : g2 port map (
    a => t1,
    b => t4,
    y => t5);

  q1 <= t5;
  q2 <= t5;
  q3 <= t5;


end gates;

use work.sigs.all;

entity ab is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end ab;
architecture gates of ab is
  component abslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

  end component;


begin -- gates
  u1 : abslice port map (
    a => p5,
    b => p8,
    q1 => p10,
    q2 => p12,
    q3 => tp1);


  u2 : abslice port map (
    a => p18,
    b => p20,
    q1 => p17,
    q2 => tp4,
    q3 => p15);


  u3 : abslice port map (
    a => p1,
    b => p3,
    q1 => p4,
    q2 => tp2,
    q3 => p6);


  u4 : abslice port map (
    a => p28,
    b => p26,
    q1 => p27,
    q2 => tp5,
    q3 => p25);


  u5 : abslice port map (
    a => p13,
    b => p11,
    q1 => p14,
    q2 => tp3,
    q3 => p16);


  u6 : abslice port map (
    a => p24,
    b => p23,
    q1 => tp6,
    q2 => p21,
    q3 => p19);



end gates;

