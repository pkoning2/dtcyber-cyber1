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
-- PJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pjslice is
    port (
      clk : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

end pjslice;
architecture gates of pjslice is
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
      qb : out logicsig);

  end component;

  signal tqi : logicsig;
  signal tqi2 : logicsig;

begin -- gates
  u1 : latch port map (
    clk => clk,
    d => d,
    q => tqi);

  tp <= tqi;

  u2 : inv2 port map (
    a => tqi,
    y => qb,
    y2 => tqi2);

  q1 <= tqi2;
  q2 <= tqi2;


end gates;

use work.sigs.all;

entity pj is
    port (
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pj;
architecture gates of pj is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component pjslice
    port (
      clk : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      qb : out logicsig);

  end component;

  signal b : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p9,
    b => p7,
    y2 => b);


  u2 : g2 port map (
    a => p20,
    b => p22,
    y2 => d);


  u3 : pjslice port map (
    clk => b,
    d => p8,
    q1 => p1,
    q2 => p3,
    qb => p4,
    tp => tp1);


  u4 : pjslice port map (
    clk => b,
    d => p5,
    q1 => p2,
    q2 => p6,
    qb => p11,
    tp => tp2);


  u5 : pjslice port map (
    clk => b,
    d => p12,
    q1 => p14,
    q2 => p10,
    qb => p13,
    tp => tp3);


  u6 : pjslice port map (
    clk => d,
    d => p21,
    q1 => p19,
    q2 => p17,
    qb => p15,
    tp => tp4);


  u7 : pjslice port map (
    clk => d,
    d => p24,
    q1 => p27,
    q2 => p18,
    qb => p16,
    tp => tp5);


  u8 : pjslice port map (
    clk => d,
    d => p23,
    q1 => p28,
    q2 => p26,
    qb => p25,
    tp => tp6);



end gates;

