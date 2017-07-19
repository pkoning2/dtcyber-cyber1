-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010-2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- MQ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mqslice is
    port (
      c : in  logicsig;
      clk : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig := '1';
      qb_tp : out logicsig;
      y : out logicsig);

end mqslice;
architecture gates of mqslice is
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

  component latchd2
    port (
      clk : in  logicsig;
      d : in  logicsig;
      d2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : latchd2 port map (
    clk => clk,
    d => i1,
    d2 => i2,
    q => t1);


  u2 : g2 port map (
    a => c,
    b => t1,
    y => y);


  u3 : inv port map (
    a => t1,
    y => qb_tp);



end gates;

use work.sigs.all;

entity mq is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p3_tp1 : out logicsig;
      p5 : out logicsig;
      p7_tp2 : out logicsig;
      p9 : out logicsig;
      p11_tp3 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22_tp4 : out logicsig;
      p25_tp5 : out logicsig;
      p26_tp6 : out logicsig;
      p27 : out logicsig);

end mq;
architecture gates of mq is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component mqslice
    port (
      c : in  logicsig;
      clk : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig := '1';
      qb_tp : out logicsig;
      y : out logicsig);

  end component;

  signal c : logicsig;

begin -- gates
  u1 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p2,
    i2 => p4,
    qb_tp => p3_tp1,
    y => p1);


  u2 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p6,
    i2 => p8,
    qb_tp => p7_tp2,
    y => p5);


  u3 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p10,
    i2 => p12,
    qb_tp => p11_tp3,
    y => p9);


  u4 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p23,
    qb_tp => p22_tp4,
    y => p20);


  u5 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p24,
    qb_tp => p25_tp5,
    y => p21);


  u6 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p28,
    qb_tp => p26_tp6,
    y => p27);


  u7 : inv port map (
    a => p16,
    y => c);

  p18 <= c;
  
  u8 : inv port map (
    a => p17,
    y => p15);



end gates;

