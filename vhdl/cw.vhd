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
-- CW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cw is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  coaxsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp4 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig);

end cw;
architecture gates of cw is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
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

  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p19,
    y => t1);


  u2 : rsflop port map (
    r => p20,
    s => t1,
    q => t2);

  tp6 <= t2;

  u3 : g5 port map (
    a => p18,
    b => p10,
    c => p3,
    d => p2,
    e => t2,
    y => p17,
    y2 => t3);


  u4 : inv port map (
    a => t3,
    y => p12);


  u5 : g5 port map (
    a => t3,
    b => p1,
    c => t2,
    d => p5,
    e => p7,
    y => a);


  u6 : rsflop port map (
    r => p9,
    s => a,
    q => t4,
    qb => b);

  p8 <= t4;
  tp1 <= t4;

  u7 : g2 port map (
    a => p4,
    b => t4,
    y => p6);


  u8 : g2 port map (
    a => a,
    b => b,
    y => t5);

  p14 <= t5;
  p15 <= t5;
  p16 <= t5;
  tp2 <= t5;

  u9 : g2 port map (
    a => t5,
    b => p13,
    y => p11);


  u10 : g6 port map (
    a => p21,
    b => p23,
    c => p26,
    d => p25,
    e => p28,
    f => p27,
    y => tp4,
    y2 => t6);

  p22 <= t6;
  p24 <= t6;


end gates;

