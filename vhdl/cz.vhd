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
-- CZ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity czslice is
    port (
      clk : in  logicsig;
      d : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig := '1';
      q1 : out logicsig;
      q2_tp : out logicsig;
      q3_q4 : out logicsig);

end czslice;
architecture gates of czslice is
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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => i,
    y => t1);

  q1 <= t1;

  u3 : latch port map (
    clk => clk,
    d => t1,
    q => t3);

  q2_tp <= t3;

  u4 : g2 port map (
    a => d,
    b => t3,
    y => t4);


  u5 : g2 port map (
    a => t4,
    b => i2,
    y => q3_q4);



end gates;

use work.sigs.all;

entity cz is
    port (
      p3 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p26 : in  logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p5_p7 : out logicsig;
      p8_p9 : out logicsig;
      p11_p24 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25_p27 : out logicsig;
      p28 : out logicsig);

end cz;
architecture gates of cz is
  component czslice
    port (
      clk : in  logicsig;
      d : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig := '1';
      q1 : out logicsig;
      q2_tp : out logicsig;
      q3_q4 : out logicsig);

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

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t3_tp6 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t5_tp3 : logicsig;
  signal t6 : logicsig;
  signal t6_tp2 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : czslice port map (
    clk => p18,
    d => '1',
    i => p26,
    i2 => t1,
    q1 => p28,
    q2_tp => tp5,
    q3_q4 => p25_p27);


  u2 : czslice port map (
    clk => p18,
    d => d,
    i => p22,
    i2 => t2,
    q1 => p23,
    q2_tp => t3,
    q3_q4 => p11_p24);

  t3_tp6 <= t3;

  u3 : g2 port map (
    a => t3,
    b => c,
    y => t1);


  u4 : czslice port map (
    clk => p18,
    d => d,
    i => p20,
    i2 => t4,
    q1 => p21,
    q2_tp => t5,
    q3_q4 => p8_p9);

  t5_tp3 <= t5;

  u5 : g2 port map (
    a => t5,
    b => c,
    y => t2);


  u6 : czslice port map (
    clk => p18,
    d => d,
    i => p3,
    q1 => p1,
    q2_tp => t6,
    q3_q4 => p5_p7);

  t6_tp2 <= t6;

  u7 : g2 port map (
    a => t6,
    b => c,
    y => t4);


  u9 : latch port map (
    clk => p18,
    d => p19,
    q => t8);


  u10 : inv port map (
    a => t8,
    y => c);



end gates;

