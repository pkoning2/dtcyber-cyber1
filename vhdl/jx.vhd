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
-- JX module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jxslice is
    port (
      i1 : in  logicsig;
      m : in  logicsig;
      clk : in  logicsig;
      t : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig);

end jxslice;
architecture gates of jxslice is
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
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
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

  q2 <= t2;
  tp <= t2;

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
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
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
      i1 : in  logicsig;
      m : in  logicsig;
      clk : in  logicsig;
      t : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig);

  end component;

  signal m : logicsig;
  signal n : logicsig;
  signal t : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : jxslice port map (
    i1 => p5,
    m => m,
    clk => p17,
    t => t,
    q1 => p7,
    q2 => p4,
    q3 => p1,
    q4 => p2,
    q5 => p3,
    tp => tp1);

  p20 <= m;
  tp4 <= m;

  u2 : jxslice port map (
    i1 => p11,
    m => m,
    clk => p17,
    t => t,
    q1 => p13,
    q2 => p14,
    q3 => p15,
    q4 => p18,
    q5 => p16,
    tp => tp3);


  u3 : jxslice port map (
    i1 => p24,
    m => m,
    clk => p17,
    t => t,
    q1 => p22,
    q2 => p25,
    q3 => p28,
    q4 => p27,
    q5 => p26,
    tp => tp6);


  u4 : inv2 port map (
    a => p17,
    y2 => t);


  u5 : g3 port map (
    a => m,
    b => t,
    c => p6,
    y => t1);

  p10 <= t1;
  tp2 <= t1;

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
    y => t2);

  p19 <= t2;
  tp5 <= t2;

  u9 : g3 port map (
    a => p23,
    b => t,
    c => n,
    y => p21);



end gates;

