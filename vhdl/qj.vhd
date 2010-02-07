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
-- QJ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qjslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      r : in  logicsig;
      s : in  coaxsig;
      tp : out logicsig;
      q : out logicsig;
      q2 : out coaxsig);

end qjslice;
architecture gates of qjslice is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => s,
    y => t1);


  u2 : rsflop port map (
    r => r,
    s => t1,
    q => q);


  u3 : g2 port map (
    a => a,
    b => b,
    y => t3);

  tp <= t3;

  u4 : cxdriver port map (
    a => t3,
    y => q2);



end gates;

use work.sigs.all;

entity qj is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  coaxsig;
      p9 : in  coaxsig;
      p10 : in  coaxsig;
      p11 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  coaxsig;
      p22 : in  coaxsig;
      p23 : in  coaxsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out coaxsig;
      p5 : out coaxsig;
      p7 : out coaxsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p24 : out coaxsig;
      p26 : out coaxsig;
      p28 : out coaxsig);

end qj;
architecture gates of qj is
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

  component qjslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      r : in  logicsig;
      s : in  coaxsig;
      tp : out logicsig;
      q : out logicsig;
      q2 : out coaxsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p18,
    y2 => c);


  u2 : inv2 port map (
    a => c,
    y2 => t1);


  u3 : inv port map (
    a => t1,
    y => a);


  u4 : qjslice port map (
    a => p11,
    b => c,
    r => a,
    s => p23,
    q => p19,
    q2 => p7,
    tp => tp1);


  u5 : qjslice port map (
    a => p6,
    b => c,
    r => a,
    s => p22,
    q => p17,
    q2 => p5,
    tp => tp2);


  u6 : qjslice port map (
    a => p4,
    b => c,
    r => a,
    s => p21,
    q => p15,
    q2 => p3,
    tp => tp3);


  u7 : qjslice port map (
    a => p27,
    b => c,
    r => a,
    s => p10,
    q => p16,
    q2 => p28,
    tp => tp4);


  u8 : qjslice port map (
    a => p25,
    b => c,
    r => a,
    s => p9,
    q => p14,
    q2 => p26,
    tp => tp5);


  u9 : qjslice port map (
    a => p20,
    b => c,
    r => a,
    s => p8,
    q => p12,
    q2 => p24,
    tp => tp6);



end gates;

