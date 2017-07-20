-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- CQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cqslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      q1_q2 : out logicsig;
      q3_q4_q5_tp : out logicsig;
      q6 : out logicsig);

end cqslice;
architecture gates of cqslice is
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

begin -- gates
  u1 : inv port map (
    a => i1,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => a,
    y => t2);


  u3 : inv2 port map (
    a => i2,
    y => t3,
    y2 => t4);

  q3_q4_q5_tp <= t4;

  u4 : inv port map (
    a => t4,
    y => t5);


  u5 : g2 port map (
    a => t5,
    b => c,
    y => q6);


  u6 : g2 port map (
    a => b,
    b => t3,
    y => t6);


  u7 : g2 port map (
    a => t2,
    b => t6,
    y2 => q1_q2);



end gates;

use work.sigs.all;

entity cq is
    port (
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p1_p3_p5_tp1 : out logicsig;
      p2_p4 : out logicsig;
      p6 : out logicsig;
      p8_p10_tp2 : out logicsig;
      p13_p15_p17_tp5 : out logicsig;
      p19 : out logicsig;
      p23 : out logicsig;
      p24_p26_p28_tp6 : out logicsig;
      p25_p27 : out logicsig);

end cq;
architecture gates of cq is
  component cqslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      q1_q2 : out logicsig;
      q3_q4_q5_tp : out logicsig;
      q6 : out logicsig);

  end component;

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

begin -- gates
  u1 : cqslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p9,
    i2 => p7,
    q1_q2 => p2_p4,
    q3_q4_q5_tp => p1_p3_p5_tp1,
    q6 => p6);


  u2 : cqslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p18,
    i2 => p16,
    q1_q2 => p8_p10_tp2,
    q3_q4_q5_tp => p13_p15_p17_tp5,
    q6 => p19);


  u4 : cqslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p20,
    i2 => p22,
    q1_q2 => p25_p27,
    q3_q4_q5_tp => p24_p26_p28_tp6,
    q6 => p23);


  u5 : inv port map (
    a => p14,
    y => c);


  u6 : inv2 port map (
    a => p11,
    y => t2,
    y2 => b);


  u7 : inv port map (
    a => p21,
    y => t3);


  u8 : g2 port map (
    a => t2,
    b => t3,
    y2 => a);



end gates;

