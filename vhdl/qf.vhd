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
-- QF module, rev D -- shift network
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      ca : in  logicsig;
      cb : in  logicsig;
      cc : in  logicsig;
      tp : out logicsig;
      ya : out logicsig;
      yb : out logicsig;
      yc : out logicsig);

end qfslice;
architecture gates of qfslice is
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

begin -- gates
  u1 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t1);

  tp <= t1;

  u2 : g2 port map (
    a => ca,
    b => t1,
    y => ya);


  u3 : g2 port map (
    a => cb,
    b => t1,
    y => yb);


  u4 : g2 port map (
    a => t1,
    b => cc,
    y => yc);



end gates;

use work.sigs.all;

entity qf is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig);

end qf;
architecture gates of qf is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component qfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      ca : in  logicsig;
      cb : in  logicsig;
      cc : in  logicsig;
      tp : out logicsig;
      ya : out logicsig;
      yb : out logicsig;
      yc : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p4,
    y => a);


  u2 : inv port map (
    a => p2,
    y => b);


  u3 : inv port map (
    a => p13,
    y => c);


  u4 : inv port map (
    a => p15,
    y => d);


  u5 : inv port map (
    a => p27,
    y => e);


  u6 : inv port map (
    a => p28,
    y => f);


  u7 : qfslice port map (
    a => p21,
    b => p23,
    c => p25,
    ca => a,
    cb => b,
    cc => c,
    tp => tp5,
    ya => p8,
    yb => t1,
    yc => t2);


  u8 : qfslice port map (
    a => p3,
    b => p5,
    c => p7,
    ca => a,
    cb => b,
    cc => c,
    tp => tp6,
    ya => t3,
    yb => t4,
    yc => t5);


  u9 : qfslice port map (
    a => p17,
    b => p19,
    c => p9,
    ca => a,
    cb => b,
    cc => c,
    tp => tp4,
    ya => t6,
    yb => t7,
    yc => p18);


  u10 : qfslice port map (
    a => p1,
    b => t1,
    c => t3,
    ca => f,
    cb => d,
    cc => e,
    tp => tp1,
    ya => p12,
    yb => p10,
    yc => p20);


  u11 : qfslice port map (
    a => t2,
    b => t4,
    c => t6,
    ca => f,
    cb => d,
    cc => e,
    tp => tp2,
    ya => p6,
    yb => p16,
    yc => p14);


  u12 : qfslice port map (
    a => t5,
    b => t7,
    c => p11,
    ca => f,
    cb => d,
    cc => e,
    tp => tp3,
    ya => p24,
    yb => p26,
    yc => p22);



end gates;

