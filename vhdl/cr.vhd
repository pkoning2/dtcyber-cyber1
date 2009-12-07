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
      a : in  logicsig;
      in1 : in  logicsig;
      tp : out coaxsig;
      y : out coaxsig);

end crslice;
architecture gates of crslice is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => in1,
    b => a,
    y => t1);

  tp <= t1;

  u2 : cxdriver port map (
    a => t1,
    y => y);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity cr is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out coaxsig;
      p6 : out coaxsig;
      p8 : out coaxsig;
      p10 : out coaxsig;
      p12 : out coaxsig;
      p19 : out coaxsig;
      p21 : out coaxsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out coaxsig);

end cr;
architecture gates of cr is
  component crslice
    port (
      a : in  logicsig;
      in1 : in  logicsig;
      tp : out coaxsig;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal t1 : coaxsig;

begin -- gates
  u1 : crslice port map (
    a => a,
    in1 => p5,
    tp => tp1,
    y => p12);

  tp2 <= a;

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

  p23 <= t1;
  p25 <= t1;

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


  u10 : g2 port map (
    a => p14,
    b => p17,
    y2 => a);



end gates;

