-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- MO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity moslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end moslice;
architecture gates of moslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => t1,
    y => t2);


  u2 : g2 port map (
    a => b,
    b => c,
    y => t1,
    y2 => t3);


  u3 : g2 port map (
    a => t3,
    b => d,
    y => t4);


  u4 : g2 port map (
    a => t2,
    b => t4,
    y => y1);


  u5 : g2 port map (
    a => d,
    b => t1,
    y => y2);



end gates;

use work.sigs.all;

entity mo is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p1_tp2 : out logicsig;
      p9_tp1 : out logicsig;
      p12_tp3 : out logicsig;
      p16 : out logicsig;
      p20_tp5 : out logicsig;
      p21_tp4 : out logicsig;
      p28_tp6 : out logicsig);

end mo;
architecture gates of mo is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component moslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p10,
    b => p6,
    y => t1);


  u2 : g2 port map (
    a => p7,
    b => p8,
    y => t2);


  u3 : g2 port map (
    a => t1,
    b => t2,
    y => p9_tp1);


  u4 : g2 port map (
    a => p6,
    b => p8,
    y => t3);


  u5 : moslice port map (
    a => p3,
    b => t3,
    c => p5,
    d => p4,
    y1 => p1_tp2,
    y2 => t4);


  u6 : moslice port map (
    a => p25,
    b => t4,
    c => p24,
    d => p26,
    y1 => p28_tp6,
    y2 => t5);


  u7 : moslice port map (
    a => p22,
    b => t5,
    c => p23,
    d => p18,
    y1 => p21_tp4,
    y2 => t6);


  u8 : moslice port map (
    a => p19,
    b => t6,
    c => p17,
    d => p15,
    y1 => p20_tp5,
    y2 => t7);


  u9 : moslice port map (
    a => p14,
    b => t7,
    c => p13,
    d => p11,
    y1 => p12_tp3,
    y2 => p16);



end gates;

