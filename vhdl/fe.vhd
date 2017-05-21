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
-- FE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity feslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      tp : out logicsig;
      c : out logicsig;
      y : out logicsig);

end feslice;
architecture gates of feslice is
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

begin -- gates
  u1 : inv port map (
    a => a,
    y => t1);

  c <= t1;

  u2 : inv port map (
    a => b,
    y => t2);

  tp <= t2;

  u3 : g2 port map (
    a => a,
    b => t2,
    y => t3);


  u4 : g2 port map (
    a => t1,
    b => b,
    y => t4);


  u5 : g2 port map (
    a => t3,
    b => t4,
    y => y);



end gates;

use work.sigs.all;

entity fe is
    port (
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      p2 : out logicsig;
      p12_tp1 : out logicsig;
      p17 : out logicsig;
      p22_tp4 : out logicsig;
      p28_tp6 : out logicsig);

end fe;
architecture gates of fe is
  component feslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      tp : out logicsig;
      c : out logicsig;
      y : out logicsig);

  end component;

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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => a,
    b => p6,
    c => p8,
    y => t1);


  u2 : inv port map (
    a => p21,
    y => t2);


  u3 : g2 port map (
    a => a,
    b => t2,
    y => t3);


  u4 : g3 port map (
    a => p11,
    b => t1,
    c => t3,
    y => t4);

  tp2 <= t4;

  u5 : feslice port map (
    a => p10,
    b => t4,
    c => c,
    y => p12_tp1);


  u6 : g2 port map (
    a => p8,
    b => p6,
    y => t5);


  u7 : g2 port map (
    a => p21,
    b => t5,
    y => t6);

  tp3 <= t6;

  u8 : feslice port map (
    a => p26,
    b => t6,
    c => a,
    y => p22_tp4);


  u9 : feslice port map (
    a => p24,
    b => p25,
    c => b,
    tp => tp5,
    y => p28_tp6);


  u10 : inv2 port map (
    a => p20,
    y => t7,
    y2 => t8);

  p17 <= t8;

  u11 : g4 port map (
    a => t7,
    b => c,
    c => a,
    d => b,
    y => t9);


  u12 : inv port map (
    a => c,
    y => t10);


  u13 : inv port map (
    a => a,
    y => t11);


  u14 : inv port map (
    a => b,
    y => t12);


  u15 : g4 port map (
    a => t8,
    b => t11,
    c => t12,
    d => t10,
    y => t13);


  u16 : g2 port map (
    a => t9,
    b => t13,
    y => p2);



end gates;

