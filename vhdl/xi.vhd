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
-- XI module -- 6612 keyboard input buffers
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity xi is
    port (
      p1 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      p3_tp1 : out logicsig;
      p6_tp2 : out logicsig;
      p7_tp3 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p22_tp4 : out logicsig;
      p23_tp5 : out logicsig;
      p26_tp6 : out logicsig);

end xi;
architecture gates of xi is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component latchr
    port (
      clk : in  logicsig;
      d : in  logicsig;
      r : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal c : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p21,
    b => p11,
    y2 => c);


  u2 : inv2 port map (
    a => p19,
    y2 => p17);


  u3 : inv2 port map (
    a => p12,
    y2 => p14);


  u4 : latchr port map (
    clk => p9,
    d => p1,
    r => c,
    q => p3_tp1);


  u5 : latchr port map (
    clk => p20,
    d => p24,
    r => c,
    q => p22_tp4);


  u6 : latchr port map (
    clk => p9,
    d => p4,
    r => c,
    q => p6_tp2);


  u7 : latchr port map (
    clk => p20,
    d => p25,
    r => c,
    q => p23_tp5);


  u8 : latchr port map (
    clk => p9,
    d => p5,
    r => c,
    q => p7_tp3);


  u9 : latchr port map (
    clk => p20,
    d => p28,
    r => c,
    q => p26_tp6);



end gates;

