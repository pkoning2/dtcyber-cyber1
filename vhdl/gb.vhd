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
-- GB module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gbslice is
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      y : out std_logic);

end gbslice;
architecture gates of gbslice is
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

  component g4
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component inv2
    port (
      a : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;
  signal t7 : std_logic;
  signal t8 : std_logic;
  signal ts : std_logic;

begin -- gates
  u1 : inv2 port map (
    a => in1,
    y => ts,
    y2 => t1);


  u2 : inv2 port map (
    a => in2,
    y => t2,
    y2 => t3);


  u3 : g2 port map (
    a => ts,
    b => a,
    y => t4);


  u4 : g3 port map (
    a => t1,
    b => t3,
    c => b,
    y => t5);


  u5 : g3 port map (
    a => c,
    b => ts,
    c => t2,
    y => t6);


  u6 : g2 port map (
    a => t2,
    b => d,
    y => t7);

  tp2 <= t7;

  u7 : g4 port map (
    a => t4,
    b => t5,
    c => t6,
    d => t7,
    y => t8);

  tp1 <= t8;
  y <= t8;


end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gb is
    port (
      p3 : in  std_logic;
      p5 : in  std_logic;
      p10 : in  std_logic;
      p15 : in  std_logic;
      p16 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p21 : in  std_logic;
      p25 : in  std_logic;
      p27 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p8 : out std_logic;
      p13 : out std_logic;
      p23 : out std_logic);

end gb;
architecture gates of gb is
  component gbslice
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      y : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;

begin -- gates
  u1 : gbslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p3,
    in2 => p5,
    tp1 => tp1,
    tp2 => tp2,
    y => p8);


  u2 : gbslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p21,
    in2 => p10,
    tp1 => tp5,
    y => p13);


  u3 : gbslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p27,
    in2 => p25,
    tp1 => tp6,
    y => p23);


  u4 : inv port map (
    a => p16,
    y => a);


  u5 : inv port map (
    a => p18,
    y => b);


  u6 : inv port map (
    a => p19,
    y => c);


  u7 : inv port map (
    a => p15,
    y => d);



end gates;

