-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- IQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity iq is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p18 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end iq;
architecture gates of iq is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => p15,
    b => p13,
    c => p17,
    y => t1);

  tp1 <= t1;

  u2 : g2 port map (
    a => t1,
    b => p16,
    y => t2);

  p14 <= t2;
  tp2 <= t2;

  u3 : g2 port map (
    a => t1,
    b => p10,
    y => p12);


  u4 : g2 port map (
    a => t1,
    b => p7,
    y => p8);


  u5 : g2 port map (
    a => t1,
    b => p3,
    y => p6);


  u6 : g2 port map (
    a => t1,
    b => p2,
    y => p4);


  u7 : g2 port map (
    a => t1,
    b => p19,
    y => t3);

  p18 <= t3;
  tp6 <= t3;

  u8 : g2 port map (
    a => t1,
    b => p20,
    y => p22);


  u9 : g2 port map (
    a => t1,
    b => p25,
    y => p26);


  u10 : g2 port map (
    a => p23,
    b => t1,
    y => p24);


  u11 : g2 port map (
    a => p27,
    b => t1,
    y => p28);



end gates;

