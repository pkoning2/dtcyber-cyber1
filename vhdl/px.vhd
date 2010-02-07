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
-- PX module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity px is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  coaxsig;
      p12 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p8 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end px;
architecture gates of px is
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

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
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
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p11,
    y => t1);


  u2 : inv2 port map (
    a => p6,
    y2 => t2);


  u3 : g2 port map (
    a => p2,
    b => t2,
    y => t3);


  u4 : r2sflop port map (
    r => t3,
    r2 => p12,
    s => t1,
    q => t4,
    qb => c);

  tp1 <= t4;

  u5 : inv port map (
    a => t4,
    y => p13);


  u6 : inv port map (
    a => p4,
    y => t5);

  tp2 <= t5;

  u7 : g3 port map (
    a => c,
    b => t5,
    c => p5,
    y2 => t6);


  u8 : inv port map (
    a => p7,
    y => t7);


  u9 : g2 port map (
    a => t7,
    b => t6,
    y => d);


  u10 : inv2 port map (
    a => d,
    y2 => p28);


  u11 : g3 port map (
    a => t6,
    b => e,
    c => p9,
    y => t8);

  p17 <= t8;

  u12 : g3 port map (
    a => g,
    b => d,
    c => t8,
    y => tp6,
    y2 => p18);


  u13 : g5 port map (
    a => t5,
    b => c,
    c => p22,
    d => e,
    e => p10,
    y => g);


  u14 : g2 port map (
    a => d,
    b => g,
    y2 => p20);


  u15 : inv2 port map (
    a => g,
    y2 => p15);


  u16 : inv port map (
    a => p22,
    y => t9);


  u17 : inv port map (
    a => p24,
    y => t10);


  u18 : g2 port map (
    a => p23,
    b => t10,
    y => p27,
    y2 => e);


  u19 : g2 port map (
    a => t10,
    b => p21,
    y => p25,
    y2 => t11);


  u20 : g2 port map (
    a => t11,
    b => p26,
    y => t12);

  p19 <= t12;

  u21 : g3 port map (
    a => p10,
    b => t9,
    c => e,
    y => t13);

  p8 <= t13;

  u22 : g3 port map (
    a => g,
    b => t13,
    c => t12,
    y => tp5,
    y2 => t14);

  p3 <= t14;
  p14 <= t14;
  p16 <= t14;

  u23 : inv port map (
    a => t14,
    y => p1);



end gates;

