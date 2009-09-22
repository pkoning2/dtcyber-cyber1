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
      a : in  std_logic;
      b : in  std_logic;
      k : in  std_logic;
      k2 : in  std_logic;
      k3 : in  std_logic;
      qa : out std_logic;
      qb : out std_logic;
      y : out std_logic);

end qmslice;
architecture gates of qmslice is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g3
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal ta : std_logic;
  signal tb : std_logic;

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
      p10 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p18 : in  std_logic;
      p25 : in  std_logic;
      p26 : in  std_logic;
      p27 : in  std_logic;
      p28 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p3 : out std_logic;
      p5 : out std_logic;
      p7 : out std_logic;
      p9 : out std_logic;
      p20 : out std_logic;
      p24 : out std_logic);

end qm;
architecture gates of qm is
  component g3
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component g4
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component qmslice
    port (
      a : in  std_logic;
      b : in  std_logic;
      k : in  std_logic;
      k2 : in  std_logic;
      k3 : in  std_logic;
      qa : out std_logic;
      qb : out std_logic;
      y : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;
  signal k : std_logic;
  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;

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

