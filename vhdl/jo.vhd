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
-- JO module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jo is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p13 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end jo;
architecture gates of jo is
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

  component latch2
    port (
      clk : in  logicsig;
      clk2 : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal f : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p18,
    y => t1);


  u2 : inv2 port map (
    a => t1,
    y => a,
    y2 => b);

  tp3 <= a;

  u3 : g2 port map (
    a => p17,
    b => p19,
    y2 => c);

  tp5 <= c;

  u4 : g2 port map (
    a => a,
    b => c,
    y => p20);


  u5 : inv2 port map (
    a => c,
    y => t2,
    y2 => t4);


  u6 : g2 port map (
    a => t4,
    b => b,
    y => t5);


  u7 : g2 port map (
    a => t2,
    b => a,
    y => t3);


  u8 : g2 port map (
    a => t3,
    b => t5,
    y => t6);

  p9 <= t6;
  tp1 <= t6;

  u9 : inv2 port map (
    a => p24,
    y2 => t7);

  p23 <= t7;

  u10 : latch2 port map (
    clk => p15,
    clk2 => p16,
    d => t7,
    q => t8);

  p22 <= t8;
  tp6 <= t8;

  u11 : g2 port map (
    a => t8,
    b => f,
    y => p21);


  u12 : inv2 port map (
    a => p5,
    y2 => t9);

  p6 <= t9;

  u13 : latch2 port map (
    clk => p15,
    clk2 => p16,
    d => t9,
    q => t10);

  p7 <= t10;
  tp2 <= t10;

  u14 : g2 port map (
    a => t10,
    b => f,
    y => p8);


  u15 : inv2 port map (
    a => p14,
    y2 => t11);

  p13 <= t11;

  u16 : latch2 port map (
    clk => p15,
    clk2 => p16,
    d => t11,
    q => t12);

  p10 <= t12;
  tp4 <= t12;

  u17 : inv port map (
    a => p12,
    y => f);


  u18 : g2 port map (
    a => t12,
    b => f,
    y => p11);


  u19 : inv port map (
    a => p26,
    y => p28);


  u20 : inv port map (
    a => p27,
    y => p25);


  u21 : inv port map (
    a => p3,
    y => p1);


  u22 : inv port map (
    a => p2,
    y => p4);



end gates;

