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
-- HX module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity hx is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  coaxsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p6 : out logicsig;
      p8_tp1 : out logicsig;
      p9_tp2 : out logicsig;
      p11 : out logicsig;
      p12_tp4 : out logicsig;
      p14_tp3 : out logicsig;
      p17 : out logicsig;
      p22_p24 : out logicsig);

end hx;
architecture gates of hx is
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

  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
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
  signal t5 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p19,
    y => t1);


  u2 : rsflop port map (
    r => p20,
    s => t1,
    q => t2);

  tp6 <= t2;

  u3 : g6 port map (
    a => p5,
    b => p7,
    c => t2,
    d => p3,
    e => p1,
    f => p2,
    y => a);

  p9_tp2 <= a;

  u4 : rsflop port map (
    r => p10,
    s => a,
    q => t3,
    qb => b);

  p8_tp1 <= t3;

  u5 : g2 port map (
    a => p4,
    b => t3,
    y => p6);


  u6 : g3 port map (
    a => p15,
    b => p16,
    c => p18,
    y => p12_tp4);


  u7 : g2 port map (
    a => p16,
    b => p18,
    y => p17);


  u8 : g2 port map (
    a => a,
    b => b,
    y => t5);

  p14_tp3 <= t5;

  u9 : g2 port map (
    a => p13,
    b => t5,
    y => p11);


  u10 : g6 port map (
    a => p21,
    b => p23,
    c => p26,
    d => p25,
    e => p28,
    f => p27,
    y => tp5,
    y2 => p22_p24);



end gates;

