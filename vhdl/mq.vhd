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
      tp : out logicsig;
      qb : out logicsig;
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
  signal t2 : logicsig;

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
    y => t2);

  qb <= t2;
  tp <= t2;


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
      p23 : in  logicsig;
      p24 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
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
      tp : out logicsig;
      qb : out logicsig;
      y : out logicsig);

  end component;

  signal c : logicsig;

begin -- gates
  u1 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p2,
    i2 => p4,
    qb => p3,
    tp => tp1,
    y => p1);


  u2 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p6,
    i2 => p8,
    qb => p7,
    tp => tp2,
    y => p5);


  u3 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p10,
    i2 => p12,
    qb => p11,
    tp => tp3,
    y => p9);


  u4 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p23,
    qb => p22,
    tp => tp4,
    y => p20);


  u5 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p24,
    qb => p25,
    tp => tp5,
    y => p21);


  u6 : mqslice port map (
    c => c,
    clk => p13,
    i1 => p28,
    qb => p26,
    tp => tp6,
    y => p27);


  u7 : inv port map (
    a => p16,
    y => c);



end gates;

