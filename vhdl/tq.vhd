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
-- TQ module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tqslice is
    port (
      a : in  logicsig;
      y : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end tqslice;
architecture gates of tqslice is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => a,
    y => y,
    y2 => t1);

  y1 <= t1;
  y2 <= t1;


end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tq is
    port (
      p2 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end tq;
architecture gates of tq is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component tqslice
    port (
      a : in  logicsig;
      y : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : tqslice port map (
    a => p12,
    y => tp1,
    y1 => p9,
    y2 => p11);


  u2 : tqslice port map (
    a => p13,
    y => tp2,
    y1 => p14,
    y2 => p16);


  u3 : tqslice port map (
    a => p7,
    y => tp3,
    y1 => p8,
    y2 => p10);


  u4 : tqslice port map (
    a => p24,
    y => tp4,
    y1 => p25,
    y2 => p23);


  u5 : tqslice port map (
    a => p21,
    y => tp6,
    y1 => p22,
    y2 => p20);


  u6 : tqslice port map (
    a => p2,
    y1 => p1,
    y2 => p3);


  u7 : tqslice port map (
    a => p27,
    y1 => p28,
    y2 => p26);


  u8 : tqslice port map (
    a => p5,
    y1 => p4,
    y2 => p6);


  u10 : inv2 port map (
    a => p18,
    y => tp5,
    y2 => t1);

  p15 <= t1;
  p17 <= t1;
  p19 <= t1;


end gates;

