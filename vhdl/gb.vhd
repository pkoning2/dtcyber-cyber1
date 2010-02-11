-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- GB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gbslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      tp1_y : out logicsig;
      tp2 : out logicsig);

end gbslice;
architecture gates of gbslice is
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
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal ts : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => in1,
    y => ts,
    y2 => t1);


  u2 : inv2 port map (
    a => in2,
    y => t2,
    y2 => t3);


  u3 : g2 port map (
    a => ts,
    b => a,
    y => t4);


  u4 : g3 port map (
    a => t1,
    b => t3,
    c => b,
    y => t5);


  u5 : g3 port map (
    a => c,
    b => ts,
    c => t2,
    y => t6);


  u6 : g2 port map (
    a => t2,
    b => d,
    y => t7);

  tp2 <= t7;

  u7 : g4 port map (
    a => t4,
    b => t5,
    c => t6,
    d => t7,
    y => tp1_y);



end gates;

use work.sigs.all;

entity gb is
    port (
      p3 : in  logicsig;
      p5 : in  logicsig;
      p10 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      p8_tp1 : out logicsig;
      p13_tp5 : out logicsig;
      p23_tp6 : out logicsig);

end gb;
architecture gates of gb is
  component gbslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      tp1_y : out logicsig;
      tp2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : gbslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p3,
    in2 => p5,
    tp1_y => p8_tp1,
    tp2 => tp2);


  u2 : gbslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p21,
    in2 => p10,
    tp1_y => p13_tp5);


  u3 : gbslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p27,
    in2 => p25,
    tp1_y => p23_tp6);


  u4 : inv port map (
    a => p16,
    y => a);


  u5 : inv port map (
    a => p18,
    y => b);


  u6 : inv port map (
    a => p19,
    y => c);


  u7 : inv port map (
    a => p15,
    y => d);



end gates;

