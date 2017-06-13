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
-- JX module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jxslice is
    port (
      clk : in  logicsig;
      i1 : in  logicsig;
      m : in  logicsig;
      q1 : out logicsig;
      q2_tp : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig);

end jxslice;
architecture gates of jxslice is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
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

  signal t : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g3 port map (
    a => m,
    b => t,
    c => i1,
    y => q1);


  u3 : latch port map (
    clk => clk,
    d => i1,
    q => t2);

  q2_tp <= t2;

  u4 : inv2 port map (
    a => t2,
    y => q5,
    y2 => t3);

  q4 <= t3;

  u5 : inv port map (
    a => t3,
    y => q3);



end gates;

use work.sigs.all;

entity jx is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p17 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4_tp1 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10_tp2 : out logicsig;
      p13 : out logicsig;
      p14_tp3 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p19_tp5 : out logicsig;
      p20_tp4 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25_tp6 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end jx;
architecture gates of jx is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component jxslice
    port (
      clk : in  logicsig;
      i1 : in  logicsig;
      m : in  logicsig;
      q1 : out logicsig;
      q2_tp : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig);

  end component;

  signal m : logicsig;
  signal n : logicsig;
  signal t : logicsig;

begin -- gates
  u1 : jxslice port map (
    clk => p17,
    i1 => p5,
    m => m,
    q1 => p7,
    q2_tp => p4_tp1,
    q3 => p1,
    q4 => p2,
    q5 => p3);

  p20_tp4 <= m;

  u2 : jxslice port map (
    clk => p17,
    i1 => p11,
    m => m,
    q1 => p13,
    q2_tp => p14_tp3,
    q3 => p15,
    q4 => p18,
    q5 => p16);


  u3 : jxslice port map (
    clk => p17,
    i1 => p24,
    m => m,
    q1 => p22,
    q2_tp => p25_tp6,
    q3 => p28,
    q4 => p27,
    q5 => p26);


  u4 : inv2 port map (
    a => p17,
    y2 => t);


  u5 : g3 port map (
    a => m,
    b => t,
    c => p6,
    y => p10_tp2);


  u6 : g3 port map (
    a => p6,
    b => t,
    c => n,
    y => p8);

  p9 <= n;

  u7 : inv2 port map (
    a => p12,
    y => m,
    y2 => n);


  u8 : g3 port map (
    a => m,
    b => t,
    c => p23,
    y => p19_tp5);


  u9 : g3 port map (
    a => p23,
    b => t,
    c => n,
    y => p21);



end gates;

