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
-- MK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mkslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      clk : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y2 : out logicsig;
      yb : out logicsig);

end mkslice;
architecture gates of mkslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => b,
    b => c,
    y => t1);


  u2 : rs2flop port map (
    r => d,
    s => a,
    s2 => t1,
    q => t2);

  tp <= t2;

  u3 : g2 port map (
    a => t2,
    b => e,
    y => y);


  u4 : latch port map (
    clk => clk,
    d => t2,
    q => y2,
    qb => yb);



end gates;

use work.sigs.all;

entity mk is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p8 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig);

end mk;
architecture gates of mk is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component mkslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      clk : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y2 : out logicsig;
      yb : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv port map (
    a => p17,
    y => a);


  u2 : inv port map (
    a => p16,
    y => c);


  u3 : mkslice port map (
    a => p2,
    b => p1,
    c => a,
    clk => p14,
    d => c,
    e => p8,
    tp => tp1,
    y => p4,
    y2 => p5,
    yb => p3);


  u4 : mkslice port map (
    a => p13,
    b => p12,
    c => a,
    clk => p14,
    d => c,
    e => p8,
    tp => tp2,
    y => p11,
    y2 => p9,
    yb => p7);


  u5 : mkslice port map (
    a => p19,
    b => p21,
    c => a,
    clk => p14,
    d => c,
    e => p18,
    tp => tp5,
    y => p20,
    y2 => p22,
    yb => p23);


  u6 : mkslice port map (
    a => p27,
    b => p28,
    c => a,
    clk => p14,
    d => c,
    e => p18,
    tp => tp6,
    y => p26,
    y2 => p24,
    yb => p25);



end gates;

