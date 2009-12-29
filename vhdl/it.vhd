-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- IT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity itslice is
    port (
      a : in  logicsig;
      c : in  logicsig;
      i : in  coaxsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

end itslice;
architecture gates of itslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => i,
    y => t1);


  u2 : rsflop port map (
    r => c,
    s => t1,
    q => t2);

  tp <= t2;

  u3 : g2 port map (
    a => a,
    b => t2,
    y => q1);


  u4 : inv port map (
    a => t2,
    y => q2);



end gates;

use work.sigs.all;

entity it is
    port (
      p4 : in  coaxsig;
      p6 : in  coaxsig;
      p7 : in  coaxsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p22 : in  coaxsig;
      p25 : in  coaxsig;
      p27 : in  coaxsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end it;
architecture gates of it is
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

  component itslice
    port (
      a : in  logicsig;
      c : in  logicsig;
      i : in  coaxsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv port map (
    a => p16,
    y => a);


  u2 : inv2 port map (
    a => p12,
    y2 => c);


  u3 : itslice port map (
    a => a,
    c => c,
    i => p4,
    q1 => p3,
    q2 => p1,
    tp => tp1);


  u4 : itslice port map (
    a => a,
    c => c,
    i => p6,
    q1 => p5,
    q2 => p8,
    tp => tp2);


  u5 : itslice port map (
    a => a,
    c => c,
    i => p7,
    q1 => p10,
    q2 => p9,
    tp => tp3);


  u6 : itslice port map (
    a => a,
    c => c,
    i => p22,
    q1 => p21,
    q2 => p20,
    tp => tp4);


  u7 : itslice port map (
    a => a,
    c => c,
    i => p25,
    q1 => p24,
    q2 => p23,
    tp => tp5);


  u8 : itslice port map (
    a => a,
    c => c,
    i => p27,
    q1 => p26,
    q2 => p28,
    tp => tp6);



end gates;

