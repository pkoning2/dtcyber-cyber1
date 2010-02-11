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
-- QH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qhslice is
    port (
      a : in  logicsig;
      b : in  logicsig := '1';
      c : in  logicsig;
      d : in  logicsig := '1';
      e : in  logicsig := '1';
      tp_y : out logicsig);

end qhslice;
architecture gates of qhslice is
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

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);


  u2 : g2 port map (
    a => c,
    b => d,
    y => t2);


  u3 : g3 port map (
    a => t1,
    b => t2,
    c => e,
    y => tp_y);



end gates;

use work.sigs.all;

entity qh is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p1_tp3 : out logicsig;
      p3_tp1 : out logicsig;
      p11_tp2 : out logicsig;
      p16_tp4 : out logicsig;
      p19_tp6 : out logicsig;
      p28_tp5 : out logicsig);

end qh;
architecture gates of qh is
  component qhslice
    port (
      a : in  logicsig;
      b : in  logicsig := '1';
      c : in  logicsig;
      d : in  logicsig := '1';
      e : in  logicsig := '1';
      tp_y : out logicsig);

  end component;


begin -- gates
  u1 : qhslice port map (
    a => p10,
    c => p12,
    e => p5,
    tp_y => p3_tp1);


  u2 : qhslice port map (
    a => p7,
    c => p9,
    tp_y => p11_tp2);


  u3 : qhslice port map (
    a => p2,
    b => p4,
    c => p6,
    d => p8,
    tp_y => p1_tp3);


  u4 : qhslice port map (
    a => p13,
    b => p15,
    c => p17,
    e => p14,
    tp_y => p16_tp4);


  u5 : qhslice port map (
    a => p21,
    b => p23,
    c => p25,
    d => p27,
    e => p26,
    tp_y => p28_tp5);


  u6 : qhslice port map (
    a => p18,
    b => p20,
    c => p22,
    d => p24,
    tp_y => p19_tp6);



end gates;

