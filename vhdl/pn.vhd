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
-- PN module, rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pnslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      q_q2 : out logicsig);

end pnslice;
architecture gates of pnslice is
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

  component r2sflop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal tqbi : logicsig;
  signal tri : logicsig;
  signal tsi : logicsig;

begin -- gates
  u1 : g2 port map (
    a => s,
    b => a,
    y => tsi);


  u2 : g2 port map (
    a => r,
    b => b,
    y => tri);


  u3 : r2sflop port map (
    r => tri,
    r2 => c,
    s => tsi,
    q => tp,
    qb => tqbi);


  u4 : inv port map (
    a => tqbi,
    y => q_q2);



end gates;

use work.sigs.all;

entity pn is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p5_p15 : out logicsig;
      p8_p12 : out logicsig;
      p19_p21 : out logicsig;
      p22_p24 : out logicsig);

end pn;
architecture gates of pn is
  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component pnslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      r : in  logicsig;
      s : in  logicsig;
      tp : out logicsig;
      q_q2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p20,
    y2 => a);


  u2 : inv2 port map (
    a => p18,
    y2 => b);


  u3 : inv2 port map (
    a => p17,
    y2 => c);


  u4 : pnslice port map (
    a => a,
    b => b,
    c => c,
    r => p3,
    s => p2,
    q_q2 => p5_p15,
    tp => tp1);


  u5 : pnslice port map (
    a => a,
    b => b,
    c => c,
    r => p4,
    s => p1,
    q_q2 => p8_p12,
    tp => tp2);


  u6 : pnslice port map (
    a => a,
    b => b,
    c => c,
    r => p25,
    s => p28,
    q_q2 => p19_p21,
    tp => tp5);


  u7 : pnslice port map (
    a => a,
    b => b,
    c => c,
    r => p26,
    s => p27,
    q_q2 => p22_p24,
    tp => tp6);



end gates;

