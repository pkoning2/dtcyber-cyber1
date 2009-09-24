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
-- MD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mdslice is
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic := '1';
      in3 : in  std_logic := '1';
      tp : out std_logic;
      q : out std_logic;
      qb : out std_logic);

end mdslice;
architecture gates of mdslice is
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

  component rs2flop
    port (
      r : in  std_logic;
      s : in  std_logic;
      s2 : in  std_logic;
      q : out std_logic;
      qb : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;

begin -- gates
  u1 : g3 port map (
    a => in1,
    b => in2,
    c => a,
    y => t1);


  u2 : rs2flop port map (
    r => b,
    s => t1,
    s2 => in3,
    q => t2);

  q <= t2;
  tp <= t2;

  u3 : g2 port map (
    a => t2,
    b => c,
    y => qb);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity md is
    port (
      p4 : in  std_logic;
      p6 : in  std_logic;
      p11 : in  std_logic;
      p12 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p16 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p24 : in  std_logic;
      p26 : in  std_logic;
      p27 : in  std_logic;
      p28 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p2 : out std_logic;
      p3 : out std_logic;
      p5 : out std_logic;
      p7 : out std_logic;
      p8 : out std_logic;
      p9 : out std_logic;
      p10 : out std_logic;
      p15 : out std_logic;
      p19 : out std_logic;
      p20 : out std_logic;
      p23 : out std_logic;
      p25 : out std_logic);

end md;
architecture gates of md is
  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  component mdslice
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic := '1';
      in3 : in  std_logic := '1';
      tp : out std_logic;
      q : out std_logic;
      qb : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;

begin -- gates
  u1 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p4,
    q => p3,
    qb => p2,
    tp => tp1);


  u2 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p6,
    q => p7,
    qb => p5,
    tp => tp2);


  u3 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p12,
    q => p10,
    qb => p8,
    tp => tp3);


  u4 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p22,
    in2 => p24,
    in3 => p21,
    q => p19,
    qb => p20,
    tp => tp4);


  u5 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p16,
    in2 => p18,
    in3 => p17,
    q => p15,
    qb => p9,
    tp => tp5);


  u6 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p26,
    in2 => p28,
    in3 => p27,
    q => p25,
    qb => p23,
    tp => tp6);


  u7 : inv port map (
    a => p11,
    y => a);


  u8 : inv port map (
    a => p13,
    y => b);


  u9 : inv port map (
    a => p14,
    y => c);



end gates;

