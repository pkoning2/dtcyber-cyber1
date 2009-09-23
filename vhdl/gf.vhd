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
-- GF module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gfslice is
    port (
      f : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      k : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

end gfslice;
architecture gates of gfslice is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal ta : std_logic;

begin -- gates
  u1 : g2 port map (
    a => in1,
    b => k,
    y => t1);


  u2 : g2 port map (
    a => in2,
    b => t1,
    y => ta);


  u3 : g2 port map (
    a => ta,
    b => f,
    y => t2);

  q1 <= t2;
  q2 <= t2;
  q3 <= t2;
  tp <= t2;


end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gf is
    port (
      p1 : in  std_logic;
      p2 : in  std_logic;
      p9 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p12 : in  std_logic;
      p14 : in  std_logic;
      p16 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p20 : in  std_logic;
      p27 : in  std_logic;
      p28 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p3 : out std_logic;
      p4 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic;
      p7 : out std_logic;
      p8 : out std_logic;
      p13 : out std_logic;
      p15 : out std_logic;
      p17 : out std_logic;
      p21 : out std_logic;
      p22 : out std_logic;
      p23 : out std_logic;
      p24 : out std_logic;
      p25 : out std_logic;
      p26 : out std_logic);

end gf;
architecture gates of gf is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component gfslice
    port (
      f : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      k : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  component inv2
    port (
      a : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal f : std_logic;
  signal i : std_logic;
  signal j : std_logic;
  signal k : std_logic;
  signal t1 : std_logic;

begin -- gates
  u1 : gfslice port map (
    f => f,
    in1 => p1,
    in2 => p2,
    k => k,
    q1 => p7,
    q2 => p5,
    q3 => p3,
    tp => tp2);


  u2 : gfslice port map (
    f => f,
    in1 => p10,
    in2 => p9,
    k => k,
    q1 => p8,
    q2 => p6,
    q3 => p4,
    tp => tp1);


  u3 : gfslice port map (
    f => f,
    in1 => p28,
    in2 => p27,
    k => k,
    q1 => p22,
    q2 => p24,
    q3 => p26,
    tp => tp5);


  u4 : gfslice port map (
    f => f,
    in1 => p19,
    in2 => p20,
    k => k,
    q1 => p21,
    q2 => p23,
    q3 => p25,
    tp => tp6);


  u5 : gfslice port map (
    f => f,
    in1 => p11,
    in2 => p12,
    k => k,
    q1 => p17,
    q2 => p15,
    q3 => p13,
    tp => tp3);


  u6 : inv2 port map (
    a => p14,
    y2 => k);


  u7 : inv port map (
    a => k,
    y => t1);


  u8 : g2 port map (
    a => p16,
    b => t1,
    y => j);

  tp4 <= j;

  u9 : g2 port map (
    a => p18,
    b => p16,
    y => i);


  u10 : g2 port map (
    a => i,
    b => j,
    y => f);



end gates;

