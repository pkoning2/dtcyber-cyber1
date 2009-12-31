-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
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
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig;
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
  signal t7 : logicsig;

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

  q3 <= t4;
  q4 <= t4;
  q5 <= t4;
  tp <= t4;

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
    y2 => t7);

  q1 <= t7;
  q2 <= t7;


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
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end cq;
architecture gates of cq is
  component cqslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig;
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
    q1 => p2,
    q2 => p4,
    q3 => p5,
    q4 => p3,
    q5 => p1,
    q6 => p6,
    tp => tp1);


  u2 : cqslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p18,
    i2 => p16,
    q1 => p8,
    q2 => p10,
    q3 => p13,
    q4 => p15,
    q5 => p17,
    q6 => p19,
    tp => t1);


  u3 : inv port map (
    a => t1,
    y => tp5);


  u4 : cqslice port map (
    a => a,
    b => b,
    c => c,
    i1 => p20,
    i2 => p22,
    q1 => p27,
    q2 => p25,
    q3 => p24,
    q4 => p26,
    q5 => p28,
    q6 => p23,
    tp => tp6);


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

