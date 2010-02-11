-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- AH module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ah is
    port (
      p2 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp6 : out logicsig;
      p3_p7 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p22 : out logicsig;
      p23_tp5 : out logicsig;
      p24_tp4 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end ah;
architecture gates of ah is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
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

  component latchd2
    port (
      clk : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal n : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal x : logicsig;
  signal z : logicsig;

begin -- gates
  u1 : g3 port map (
    a => x,
    b => e,
    c => p25,
    y => p26,
    y2 => t1);


  u2 : inv port map (
    a => t1,
    y => p28);


  u3 : latchd2 port map (
    clk => p25,
    d => x,
    d2 => e,
    q => t2);

  tp6 <= t2;

  u4 : inv port map (
    a => t2,
    y => t3);


  u5 : g3 port map (
    a => p16,
    b => t3,
    c => z,
    y => p22,
    y2 => t4);

  p18 <= t4;

  u6 : inv port map (
    a => p14,
    y => t5);


  u7 : g2 port map (
    a => t5,
    b => t4,
    y => p23_tp5);


  u8 : inv port map (
    a => p19,
    y => t7);


  u9 : inv port map (
    a => p11,
    y => t8);


  u10 : g5 port map (
    a => x,
    b => t7,
    c => t8,
    d => b,
    e => n,
    y2 => d);

  p15 <= n;

  u11 : g2 port map (
    a => g,
    b => d,
    y => p6);


  u12 : g2 port map (
    a => d,
    b => p20,
    y => z);


  u13 : latch port map (
    clk => p20,
    d => d,
    q => a,
    qb => b);

  tp3 <= a;

  u14 : inv2 port map (
    a => a,
    y2 => t10);


  u15 : latch port map (
    clk => p9,
    d => t10,
    q => t11);

  tp1 <= t11;

  u16 : inv port map (
    a => t11,
    y => p8);


  u17 : inv2 port map (
    a => p17,
    y => x,
    y2 => p24_tp4);


  u18 : inv2 port map (
    a => p10,
    y => e,
    y2 => n);


  u19 : inv2 port map (
    a => p2,
    y => tp2,
    y2 => g);


  u20 : g2 port map (
    a => d,
    b => g,
    y => p3_p7);



end gates;

