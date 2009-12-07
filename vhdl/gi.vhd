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
-- GI module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gi is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end gi;
architecture gates of gi is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p2,
    y2 => p1);


  u2 : inv2 port map (
    a => p3,
    y2 => p4);


  u3 : inv2 port map (
    a => p5,
    y2 => p6);


  u4 : inv2 port map (
    a => p7,
    y2 => p8);


  u5 : inv2 port map (
    a => p9,
    y2 => t1);

  p10 <= t1;
  tp3 <= t1;

  u6 : inv2 port map (
    a => p11,
    y2 => t2);

  p12 <= t2;
  tp2 <= t2;

  u7 : inv2 port map (
    a => p13,
    y2 => t3);

  p14 <= t3;
  tp1 <= t3;

  u8 : inv2 port map (
    a => p16,
    y2 => t4);

  p15 <= t4;
  tp6 <= t4;

  u9 : inv2 port map (
    a => p18,
    y2 => t5);

  p17 <= t5;
  tp5 <= t5;

  u10 : inv2 port map (
    a => p20,
    y2 => t6);

  p19 <= t6;
  tp4 <= t6;

  u11 : inv2 port map (
    a => p22,
    y2 => p21);


  u12 : inv2 port map (
    a => p24,
    y2 => p23);


  u13 : inv2 port map (
    a => p26,
    y2 => p25);


  u14 : inv2 port map (
    a => p27,
    y2 => p28);



end gates;

