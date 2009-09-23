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
-- MF module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mfslice is
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic;
      q4 : out std_logic;
      q5 : out std_logic;
      q6 : out std_logic);

end mfslice;
architecture gates of mfslice is
  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
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

begin -- gates
  u1 : inv port map (
    a => in1,
    y => t1);


  u2 : inv port map (
    a => in2,
    y => t2);


  u3 : g2 port map (
    a => in1,
    b => t2,
    y => t3);


  u4 : g2 port map (
    a => in2,
    b => t1,
    y => t4);


  u5 : g2 port map (
    a => t3,
    b => t4,
    y => tp,
    y2 => t5);

  q1 <= t5;
  q2 <= t5;

  u6 : g2 port map (
    a => t5,
    b => a,
    y => q3);


  u7 : g2 port map (
    a => t5,
    b => b,
    y => q4);


  u8 : g2 port map (
    a => t5,
    b => c,
    y => q5);


  u9 : g2 port map (
    a => t5,
    b => d,
    y => q6);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity mf is
    port (
      p7 : in  std_logic;
      p8 : in  std_logic;
      p9 : in  std_logic;
      p10 : in  std_logic;
      p15 : in  std_logic;
      p17 : in  std_logic;
      p19 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      tp1 : out std_logic;
      tp3 : out std_logic;
      tp4 : out std_logic;
      tp5 : out std_logic;
      p1 : out std_logic;
      p2 : out std_logic;
      p3 : out std_logic;
      p4 : out std_logic;
      p5 : out std_logic;
      p6 : out std_logic;
      p11 : out std_logic;
      p12 : out std_logic;
      p13 : out std_logic;
      p14 : out std_logic;
      p16 : out std_logic;
      p18 : out std_logic;
      p23 : out std_logic;
      p24 : out std_logic;
      p25 : out std_logic;
      p26 : out std_logic;
      p27 : out std_logic;
      p28 : out std_logic);

end mf;
architecture gates of mf is
  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  component mfslice
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic;
      q4 : out std_logic;
      q5 : out std_logic;
      q6 : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;

begin -- gates
  u1 : inv port map (
    a => p20,
    y => a);

  tp5 <= a;

  u2 : inv port map (
    a => p19,
    y => b);

  tp3 <= b;

  u3 : inv port map (
    a => p17,
    y => c);

  p2 <= c;

  u4 : inv port map (
    a => p15,
    y => d);


  u5 : mfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p7,
    in2 => p8,
    q1 => p6,
    q2 => p5,
    q3 => p3,
    q4 => p4,
    q5 => p2,
    q6 => p1,
    tp => tp1);


  u6 : mfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p9,
    in2 => p10,
    q1 => p11,
    q2 => p13,
    q3 => p18,
    q4 => p16,
    q5 => p14,
    q6 => p12,
    tp => tp4);


  u7 : mfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p22,
    in2 => p21,
    q1 => p23,
    q2 => p24,
    q3 => p25,
    q4 => p26,
    q5 => p27,
    q6 => p28);



end gates;

