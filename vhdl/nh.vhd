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
-- NH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity nhslice is
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      i6 : in  logicsig;
      i7 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig;
      y5 : out logicsig;
      y6 : out logicsig);

end nhslice;
architecture gates of nhslice is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal ta : logicsig;
  signal tb : logicsig;
  signal te : logicsig;

begin -- gates
  u1 : inv port map (
    a => i1,
    y => t1);


  u2 : inv port map (
    a => i3,
    y => te);


  u3 : inv port map (
    a => i5,
    y => ta);


  u4 : inv port map (
    a => i6,
    y => tb);


  u5 : g2 port map (
    a => i2,
    b => te,
    y => t3);


  u6 : g3 port map (
    a => te,
    b => ta,
    c => i4,
    y => t4);


  u7 : g3 port map (
    a => t1,
    b => t5,
    c => t4,
    y => y1);


  u8 : g2 port map (
    a => i2,
    b => te,
    y => t5);


  u9 : g2 port map (
    a => i4,
    b => ta,
    y => y2);


  u10 : g3 port map (
    a => ta,
    b => tb,
    c => i7,
    y => y3);


  u11 : g2 port map (
    a => tb,
    b => i7,
    y => y4);


  u12 : g3 port map (
    a => ta,
    b => tb,
    c => te,
    y => y5,
    y2 => y6);



end gates;

use work.sigs.all;

entity nh is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p5_tp2 : out logicsig;
      p11_tp1 : out logicsig;
      p13 : out logicsig;
      p14_tp3 : out logicsig;
      p16 : out logicsig;
      p17_tp5 : out logicsig;
      p18_tp6 : out logicsig;
      p23 : out logicsig;
      p26_tp4 : out logicsig;
      p28 : out logicsig);

end nh;
architecture gates of nh is
  component nhslice
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      i6 : in  logicsig;
      i7 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig;
      y5 : out logicsig;
      y6 : out logicsig);

  end component;


begin -- gates
  u1 : nhslice port map (
    i1 => p10,
    i2 => p8,
    i3 => p9,
    i4 => p6,
    i5 => p7,
    i6 => p2,
    i7 => p3,
    y1 => p11_tp1,
    y2 => p4,
    y3 => p5_tp2,
    y4 => p1,
    y5 => p14_tp3,
    y6 => p13);


  u2 : nhslice port map (
    i1 => p19,
    i2 => p21,
    i3 => p20,
    i4 => p25,
    i5 => p22,
    i6 => p27,
    i7 => p24,
    y1 => p18_tp6,
    y2 => p23,
    y3 => p26_tp4,
    y4 => p28,
    y5 => p17_tp5,
    y6 => p16);



end gates;

