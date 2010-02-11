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
-- QI module -- I/O cable transceiver
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qislice is
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  coaxsig;
      tp : out logicsig;
      q : out coaxsig;
      q1_q2 : out logicsig;
      qb : out logicsig);

end qislice;
architecture gates of qislice is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

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

  component inv2
    port (
      a : in  logicsig;
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => s,
    y => t1);


  u2 : rsflop port map (
    r => r,
    s => t1,
    q => t2);

  tp <= t2;

  u3 : inv2 port map (
    a => t2,
    y => qb,
    y2 => t3);

  q1_q2 <= t3;

  u4 : g2 port map (
    a => t3,
    b => e,
    y => t4);


  u5 : cxdriver port map (
    a => t4,
    y => q);



end gates;

use work.sigs.all;

entity qi is
    port (
      p3 : in  coaxsig;
      p12 : in  logicsig;
      p13 : in  coaxsig;
      p16 : in  coaxsig;
      p26 : in  coaxsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out coaxsig;
      p5_p7 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p9_p11 : out logicsig;
      p14 : out coaxsig;
      p15 : out coaxsig;
      p18_p20 : out logicsig;
      p22_p24 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p28 : out coaxsig);

end qi;
architecture gates of qi is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component qislice
    port (
      e : in  logicsig;
      r : in  logicsig;
      s : in  coaxsig;
      tp : out logicsig;
      q : out coaxsig;
      q1_q2 : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p12,
    y => t1,
    y2 => b);


  u2 : inv2 port map (
    a => t1,
    y2 => t2);


  u3 : inv2 port map (
    a => t2,
    y2 => a);


  u4 : qislice port map (
    e => b,
    r => a,
    s => p3,
    q => p1,
    q1_q2 => p5_p7,
    qb => p6,
    tp => tp1);


  u5 : qislice port map (
    e => b,
    r => a,
    s => p13,
    q => p15,
    q1_q2 => p9_p11,
    qb => p8,
    tp => tp2);


  u6 : qislice port map (
    e => b,
    r => a,
    s => p16,
    q => p14,
    q1_q2 => p18_p20,
    qb => p23,
    tp => tp5);


  u7 : qislice port map (
    e => b,
    r => a,
    s => p26,
    q => p28,
    q1_q2 => p22_p24,
    qb => p25,
    tp => tp6);



end gates;

