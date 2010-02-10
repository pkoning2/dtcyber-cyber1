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
-- PC module rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pc is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p9 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p21 : out logicsig);

end pc;
architecture gates of pc is
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

  component latchd4
    port (
      clk : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      e : in  logicsig;
      e2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal e : logicsig;
  signal f : logicsig;
  signal q1 : logicsig;
  signal q2 : logicsig;
  signal q3 : logicsig;
  signal q4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p13,
    y => e);


  u2 : g2 port map (
    a => e,
    b => p20,
    y => f);


  u3 : latchd4 port map (
    clk => p11,
    d => p7,
    d2 => p5,
    e => e,
    e2 => f,
    q => q1);

  p14 <= q1;
  tp1 <= q1;

  u4 : latchd4 port map (
    clk => p22,
    d => p8,
    d2 => p6,
    e => e,
    e2 => f,
    q => q2);

  p9 <= q2;
  tp2 <= q2;

  u5 : latchd4 port map (
    clk => p11,
    d => p23,
    d2 => p25,
    e => e,
    e2 => f,
    q => q3);

  p18 <= q3;
  tp5 <= q3;

  u6 : latchd4 port map (
    clk => p22,
    d => p24,
    d2 => p26,
    e => e,
    e2 => f,
    q => q4);

  p21 <= q4;
  tp6 <= q4;

  p2 <= '0';
  p4 <= '0';
  p15 <= '0';
  p17 <= '0';

end gates;

