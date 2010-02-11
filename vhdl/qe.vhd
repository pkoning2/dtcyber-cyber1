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
-- QE module, rev D -- shift control
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qeslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      ya : out logicsig;
      yb : out logicsig;
      yc : out logicsig);

end qeslice;
architecture gates of qeslice is
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

begin -- gates
  u1 : inv port map (
    a => b,
    y => t1);

  yb <= t1;

  u2 : g2 port map (
    a => a,
    b => t1,
    y => ya);


  u3 : g2 port map (
    a => t1,
    b => c,
    y => yc);



end gates;

use work.sigs.all;

entity qe is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p5 : out logicsig;
      p8 : out logicsig;
      p9_p14_p22 : out logicsig;
      p13 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p21 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end qe;
architecture gates of qe is
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

  component latchd2
    port (
      clk : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component qeslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      ya : out logicsig;
      yb : out logicsig;
      yc : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : latchd2 port map (
    clk => p10,
    d => p7,
    d2 => p11,
    q => t2);


  u3 : inv port map (
    a => t2,
    y => a);

  tp2 <= a;

  u4 : g2 port map (
    a => e,
    b => t2,
    y2 => b);

  tp1 <= b;

  u5 : qeslice port map (
    a => a,
    b => p3,
    c => b,
    ya => p8,
    yc => p2);


  u6 : qeslice port map (
    a => a,
    b => p4,
    c => b,
    ya => p1,
    yc => p5);


  u7 : qeslice port map (
    a => a,
    b => p19,
    c => b,
    ya => p18,
    yc => p16);


  u8 : qeslice port map (
    a => a,
    b => p23,
    c => b,
    ya => p26,
    yc => p28);


  u9 : qeslice port map (
    a => a,
    b => p25,
    c => b,
    ya => p21,
    yc => p27);


  u10 : qeslice port map (
    a => a,
    b => p12,
    c => b,
    ya => p17,
    yb => t3,
    yc => p15);


  u11 : g2 port map (
    a => t3,
    b => f,
    y => p13);

  tp5 <= f;

  u12 : latchd2 port map (
    clk => p10,
    d => p20,
    d2 => p24,
    q => e);


  u13 : inv port map (
    a => e,
    y => f);


  u14 : inv2 port map (
    a => p6,
    y => tp6,
    y2 => p9_p14_p22);



end gates;

