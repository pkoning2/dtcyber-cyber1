-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- RE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity reslice is
    port (
      c : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      y : out logicsig);

end reslice;
architecture gates of reslice is
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

begin -- gates
  u1 : rsflop port map (
    r => r,
    s => s,
    q => t1);

  tp <= t1;

  u2 : g2 port map (
    a => c,
    b => t1,
    y => y);



end gates;

use work.sigs.all;

entity re is
    port (
      p1 : in  logicsig;
      p4 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p2 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end re;
architecture gates of re is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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

  component reslice
    port (
      c : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal e : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p10,
    y => a);


  u2 : inv2 port map (
    a => p13,
    y2 => c);


  u3 : inv port map (
    a => p17,
    y => e);


  u4 : reslice port map (
    c => c,
    r => a,
    s => p11,
    tp => tp1,
    y => p6);


  u5 : reslice port map (
    c => c,
    r => a,
    s => p18,
    tp => tp4,
    y => p7);


  u6 : reslice port map (
    c => c,
    r => a,
    s => p4,
    tp => tp2,
    y => p5);


  u7 : reslice port map (
    c => c,
    r => a,
    s => p16,
    tp => tp5,
    y => p9);


  u8 : reslice port map (
    c => c,
    r => a,
    s => p1,
    tp => tp3,
    y => p2);


  u9 : reslice port map (
    c => c,
    r => a,
    s => p15,
    tp => tp6,
    y => p12);


  u10 : inv port map (
    a => p27,
    y => t1);


  u11 : inv port map (
    a => p22,
    y => t2);


  u12 : inv port map (
    a => p19,
    y => t3);


  u13 : inv port map (
    a => p20,
    y => t4);


  u14 : g4 port map (
    a => p27,
    b => p22,
    c => p19,
    d => p20,
    y => p24);


  u15 : g2 port map (
    a => t1,
    b => e,
    y => p28);


  u16 : g2 port map (
    a => t2,
    b => e,
    y => p25);


  u17 : g2 port map (
    a => t3,
    b => e,
    y => p26);


  u18 : g2 port map (
    a => t4,
    b => e,
    y => p23);



end gates;

