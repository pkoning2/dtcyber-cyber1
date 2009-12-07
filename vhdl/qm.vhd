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
-- QM module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qmslice is
    port (
      a : in  logicsig;
      b : in  logicsig := '1';
      k : in  logicsig;
      k2 : in  logicsig := '1';
      k3 : in  logicsig := '1';
      qa : out logicsig;
      qb : out logicsig;
      y : out logicsig);

end qmslice;
architecture gates of qmslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal ta : logicsig;
  signal tb : logicsig;

begin -- gates
  u1 : inv port map (
    a => a,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => b,
    y => ta,
    y2 => tb);

  qa <= ta;
  qb <= tb;

  u3 : g3 port map (
    a => k,
    b => k2,
    c => k3,
    y => t2,
    y2 => t3);


  u4 : g2 port map (
    a => t3,
    b => tb,
    y => t4);


  u5 : g2 port map (
    a => t2,
    b => ta,
    y => t5);


  u6 : g2 port map (
    a => t4,
    b => t5,
    y => y);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qm is
    port (
      p10 : in  logicsig;
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

end qm;
architecture gates of qm is
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

  component qmslice
    port (
      a : in  logicsig;
      b : in  logicsig := '1';
      k : in  logicsig;
      k2 : in  logicsig := '1';
      k3 : in  logicsig := '1';
      qa : out logicsig;
      qb : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal k : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : qmslice port map (
    a => p13,
    b => p10,
    k => k,
    qa => a,
    qb => b,
    y => t1);

  p9 <= t1;
  tp1 <= t1;
  tp3 <= a;

  u2 : qmslice port map (
    a => p14,
    k => k,
    k2 => a,
    qa => c,
    qb => d,
    y => t2);

  p24 <= t2;
  tp2 <= c;
  tp6 <= t2;

  u3 : qmslice port map (
    a => p18,
    k => k,
    k2 => a,
    k3 => c,
    qa => e,
    qb => f,
    y => t3);

  p20 <= t3;
  tp4 <= e;
  tp5 <= t3;

  u4 : g4 port map (
    a => p26,
    b => p25,
    c => p28,
    d => p27,
    y2 => k);


  u5 : g3 port map (
    a => a,
    b => c,
    c => e,
    y2 => t4);

  p3 <= t4;
  p5 <= t4;
  p7 <= t4;


end gates;

