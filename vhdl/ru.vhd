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
-- RU module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ruslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

end ruslice;
architecture gates of ruslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => t1,
    y => y1);


  u2 : g2 port map (
    a => t2,
    b => t3,
    y => t1);


  u3 : g2 port map (
    a => t4,
    b => t6,
    y => t2);

  y2 <= t6;

  u4 : inv port map (
    a => b,
    y => t4);


  u5 : g2 port map (
    a => b,
    b => t5,
    y => t3);


  u6 : g2 port map (
    a => c,
    b => d,
    y => t6,
    y2 => t5);



end gates;

use work.sigs.all;

entity ru is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      p2_tp2 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p10_tp6 : out logicsig;
      p11_tp1 : out logicsig;
      p13 : out logicsig;
      p15_tp5 : out logicsig;
      p17 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig);

end ru;
architecture gates of ru is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component ruslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig);

  end component;


begin -- gates
  u1 : ruslice port map (
    a => p20,
    b => p19,
    c => p24,
    d => p26,
    y1 => p15_tp5,
    y2 => p17);


  u2 : ruslice port map (
    a => p20,
    b => p25,
    c => p28,
    d => p23,
    y1 => p10_tp6,
    y2 => p21);


  u3 : ruslice port map (
    a => p16,
    b => p1,
    c => p4,
    d => p3,
    y1 => p2_tp2,
    y2 => p7);


  u4 : ruslice port map (
    a => p16,
    b => p6,
    c => p14,
    d => p12,
    y1 => p11_tp1,
    y2 => p9);


  u5 : inv port map (
    a => p8,
    y => p13);


  u6 : inv port map (
    a => p18,
    y => p22);



end gates;

