-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- QA module, rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qaslice is
    port (
      clk1 : in  logicsig;
      clk2 : in  logicsig;
      d1 : in  logicsig;
      d2 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      q1 : out logicsig;
      q12 : out logicsig;
      qb2 : out logicsig;
      qb11_qb12 : out logicsig);

end qaslice;
architecture gates of qaslice is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig;
      qs : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => clk1,
    d => d1,
    q => t1);

  tp1 <= t1;

  u2 : inv2 port map (
    a => t1,
    y => qb11_qb12,
    y2 => q1);


  u3 : latch port map (
    clk => clk2,
    d => d2,
    q => t3);

  tp3 <= t3;

  u4 : g2 port map (
    a => t1,
    b => t3,
    y => q12);


  u5 : inv port map (
    a => t3,
    y => qb2);



end gates;

use work.sigs.all;

entity qa is
    port (
      p11 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p19 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3_p5 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p13_p17 : out logicsig;
      p21_p25 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p28 : out logicsig);

end qa;
architecture gates of qa is
  component qaslice
    port (
      clk1 : in  logicsig;
      clk2 : in  logicsig;
      d1 : in  logicsig;
      d2 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      q1 : out logicsig;
      q12 : out logicsig;
      qb2 : out logicsig;
      qb11_qb12 : out logicsig);

  end component;


begin -- gates
  u1 : qaslice port map (
    clk1 => p19,
    clk2 => p19,
    d1 => p11,
    d2 => p15,
    q1 => p6,
    q12 => p9,
    qb2 => p1,
    qb11_qb12 => p3_p5,
    tp1 => tp1,
    tp3 => tp3);


  u2 : qaslice port map (
    clk1 => p19,
    clk2 => p14,
    d1 => p12,
    d2 => p16,
    q1 => p2,
    q12 => p10,
    qb2 => p8,
    qb11_qb12 => p13_p17,
    tp1 => tp2,
    tp3 => tp4);


  u3 : qaslice port map (
    clk1 => p14,
    clk2 => p14,
    d1 => p27,
    d2 => p26,
    q1 => p22,
    q12 => p23,
    qb2 => p28,
    qb11_qb12 => p21_p25,
    tp1 => tp5,
    tp3 => tp6);



end gates;

