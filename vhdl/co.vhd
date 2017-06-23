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
-- CO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity coslice is
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      tp_y : out logicsig;
      y2 : out logicsig);

end coslice;
architecture gates of coslice is
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

begin -- gates
  u1 : inv2 port map (
    a => i1,
    y => t1,
    y2 => t2);


  u2 : g2 port map (
    a => t1,
    b => i2,
    y => t3);


  u3 : g2 port map (
    a => t2,
    b => i3,
    y => t4);


  u4 : g3 port map (
    a => t4,
    b => t3,
    c => a,
    y => tp_y,
    y2 => y2);



end gates;

use work.sigs.all;

entity co is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p3 : out logicsig;
      p4_tp1 : out logicsig;
      p8_tp2 : out logicsig;
      p9 : out logicsig;
      p14_tp3 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25_tp6 : out logicsig;
      p26 : out logicsig);

end co;
architecture gates of co is
  component coslice
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      tp_y : out logicsig;
      y2 : out logicsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : coslice port map (
    a => a,
    i1 => p27,
    i2 => p28,
    i3 => p24,
    tp_y => p25_tp6,
    y2 => p26);

  u2 : coslice port map (
    a => a,
    i1 => p2,
    i2 => p1,
    i3 => p5,
    tp_y => p4_tp1,
    y2 => p3);


  u3 : coslice port map (
    a => a,
    i1 => p6,
    i2 => p7,
    i3 => p11,
    tp_y => p8_tp2,
    y2 => p9);


  u4 : inv port map (
    a => p23,
    y => t1);


  u5 : inv port map (
    a => p20,
    y => t2);


  u6 : inv port map (
    a => p19,
    y => t3);


  u7 : g2 port map (
    a => t1,
    b => a,
    y => p22);


  u8 : g2 port map (
    a => t2,
    b => a,
    y => p21);


  u9 : g2 port map (
    a => t3,
    b => a,
    y => p18);


  u10 : inv2 port map (
    a => p17,
    y2 => t4);

  tp4 <= t4;

  u11 : g4 port map (
    a => p10,
    b => p12,
    c => p13,
    d => t4,
    y => p14_tp3);


  u12 : inv port map (
    a => p16,
    y => t6);


  u13 : g2 port map (
    a => t4,
    b => t6,
    y => p15,
    y2 => a);

  tp5 <= a;



end gates;

