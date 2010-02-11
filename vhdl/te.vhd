-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- TE module, rev C -- hex 3-input mux
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity teslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      e1 : in  logicsig;
      e2 : in  logicsig;
      e3 : in  logicsig;
      tp_y : out logicsig);

end teslice;
architecture gates of teslice is
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
  u1 : g2 port map (
    a => a,
    b => e1,
    y => t1);


  u2 : g2 port map (
    a => b,
    b => e2,
    y => t2);


  u3 : g2 port map (
    a => c,
    b => e3,
    y => t3);


  u4 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => tp_y);



end gates;

use work.sigs.all;

entity te is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p8_tp2 : out logicsig;
      p9_tp1 : out logicsig;
      p10_tp3 : out logicsig;
      p19_tp4 : out logicsig;
      p20_tp6 : out logicsig;
      p21_tp5 : out logicsig);

end te;
architecture gates of te is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component teslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      e1 : in  logicsig;
      e2 : in  logicsig;
      e3 : in  logicsig;
      tp_y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv port map (
    a => p14,
    y => a);


  u2 : inv port map (
    a => p15,
    y => b);


  u3 : inv port map (
    a => p16,
    y => c);


  u4 : teslice port map (
    a => p6,
    b => p4,
    c => p2,
    e1 => a,
    e2 => b,
    e3 => c,
    tp_y => p9_tp1);


  u5 : teslice port map (
    a => p5,
    b => p3,
    c => p1,
    e1 => a,
    e2 => b,
    e3 => c,
    tp_y => p8_tp2);


  u6 : teslice port map (
    a => p7,
    b => p13,
    c => p12,
    e1 => a,
    e2 => b,
    e3 => c,
    tp_y => p10_tp3);


  u7 : teslice port map (
    a => p22,
    b => p18,
    c => p17,
    e1 => a,
    e2 => b,
    e3 => c,
    tp_y => p19_tp4);


  u8 : teslice port map (
    a => p24,
    b => p26,
    c => p28,
    e1 => a,
    e2 => b,
    e3 => c,
    tp_y => p21_tp5);


  u9 : teslice port map (
    a => p23,
    b => p25,
    c => p27,
    e1 => a,
    e2 => b,
    e3 => c,
    tp_y => p20_tp6);



end gates;

