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
-- PI module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pislice is
    port (
      r : in  logicsig;
      s : in  coaxsig;
      tp : out logicsig;
      q : out logicsig;
      qb : out logicsig);

end pislice;
architecture gates of pislice is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

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
    y2 => q);



end gates;

use work.sigs.all;

entity pi is
    port (
      p5 : in  coaxsig;
      p7 : in  coaxsig;
      p10 : in  coaxsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p21 : in  coaxsig;
      p24 : in  coaxsig;
      p26 : in  coaxsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p4 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p22 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end pi;
architecture gates of pi is
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

  component pislice
    port (
      r : in  logicsig;
      s : in  coaxsig;
      tp : out logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p16,
    b => p15,
    y2 => t1);


  u2 : pislice port map (
    r => a,
    s => p5,
    q => p9,
    qb => p12,
    tp => tp1);


  u3 : pislice port map (
    r => a,
    s => p7,
    q => p1,
    qb => p4,
    tp => tp2);


  u4 : pislice port map (
    r => a,
    s => p10,
    q => p13,
    qb => p14,
    tp => tp3);


  u5 : pislice port map (
    r => a,
    s => p21,
    q => p18,
    qb => p17,
    tp => tp4);


  u6 : pislice port map (
    r => a,
    s => p24,
    q => p28,
    qb => p27,
    tp => tp5);


  u7 : pislice port map (
    r => a,
    s => p26,
    q => p22,
    qb => p19,
    tp => tp6);


  u9 : inv port map (
    a => t1,
    y => a);



end gates;

