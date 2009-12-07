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
-- RD module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity rd is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p11 : out logicsig;
      p16 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig);

end rd;
architecture gates of rd is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component rs4flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : rs4flop port map (
    r => x,
    s => p9,
    s2 => p3,
    s3 => p13,
    s4 => p1,
    q => t1);

  p11 <= t1;
  tp1 <= t1;

  u2 : rs4flop port map (
    r => x,
    s => p15,
    s2 => p17,
    s3 => p12,
    s4 => p14,
    q => t2);

  p16 <= t2;
  tp4 <= t2;

  u3 : rs2flop port map (
    r => a,
    s => p10,
    s2 => p8,
    q => t3);

  p7 <= t3;
  tp2 <= t3;

  u4 : rs4flop port map (
    r => x,
    s => p28,
    s2 => p24,
    s3 => p22,
    s4 => p20,
    q => t4);

  p26 <= t4;
  tp5 <= t4;

  u5 : rs2flop port map (
    r => a,
    s => p2,
    s2 => p4,
    q => t5);

  p6 <= t5;
  tp3 <= t5;

  u6 : rs4flop port map (
    r => x,
    s => p19,
    s2 => p21,
    s3 => p23,
    s4 => p25,
    q => t6);

  p27 <= t6;
  tp6 <= t6;

  u7 : inv port map (
    a => p5,
    y => a);


  u8 : inv port map (
    a => p18,
    y => x);



end gates;

