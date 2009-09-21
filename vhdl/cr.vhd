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
-- CR module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity crslice is
    port (
      a : in  std_logic;
      in1 : in  std_logic;
      tp : out std_logic;
      y : out coaxsig);

end crslice;
architecture gates of crslice is
  component cxdriver
    port (
      a : in  std_logic;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;

begin -- gates
  u1 : g2 port map (
    a => in1,
    b => a,
    y => t1);

  u2 : cxdriver port map (
    a => t1,
    y => y);

tp <= t1;

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity cr is
    port (
      p11 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p15 : in  std_logic;
      p16 : in  std_logic;
      p17 : in  std_logic;
      p22 : in  std_logic;
      p24 : in  std_logic;
      p26 : in  std_logic;
      p5 : in  std_logic;
      p7 : in  std_logic;
      p9 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p10 : out coaxsig;
      p12 : out coaxsig;
      p19 : out coaxsig;
      p21 : out coaxsig;
      p23 : out std_logic;
      p25 : out std_logic;
      p27 : out coaxsig;
      p4 : out coaxsig;
      p6 : out coaxsig;
      p8 : out coaxsig);

end cr;
architecture gates of cr is
  component crslice
    port (
      a : in  std_logic;
      in1 : in  std_logic;
      tp : out std_logic;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal a : std_logic;
  signal t1 : coaxsig;

begin -- gates
  u1 : crslice port map (
    a => a,
    in1 => p5,
    tp => tp1,
    y => p12);

  u10 : g2 port map (
    a => p14,
    b => p17,
    y2 => a);

  u2 : crslice port map (
    a => a,
    in1 => p26,
    tp => tp6,
    y => p19);

  u3 : crslice port map (
    a => a,
    in1 => p7,
    y => p10);

  u4 : crslice port map (
    a => a,
    in1 => p24,
    y => p21);

  u5 : crslice port map (
    a => a,
    in1 => p9,
    y => p8);

  u6 : crslice port map (
    a => a,
    in1 => p22,
    y => t1);

  u7 : crslice port map (
    a => a,
    in1 => p11,
    y => p6);

  u8 : crslice port map (
    a => a,
    in1 => p13,
    y => p4);

  u9 : crslice port map (
    a => p15,
    in1 => p16,
    tp => tp5,
    y => p27);

p23 <= t1;
p25 <= t1;
tp2 <= a;

end gates;

