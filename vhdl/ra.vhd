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
-- RA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity raslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  coaxsig;
      f : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig);

end raslice;
architecture gates of raslice is
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
  signal t2 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => e,
    y => t1);


  u2 : rsflop port map (
    r => f,
    s => t1,
    q => t2,
    qb => y);

  tp <= t2;

  u3 : g2 port map (
    a => a,
    b => t2,
    y => y1);


  u4 : g2 port map (
    a => b,
    b => t2,
    y => y2);


  u5 : g2 port map (
    a => c,
    b => t2,
    y => y3);


  u6 : g2 port map (
    a => d,
    b => t2,
    y => y4);



end gates;

use work.sigs.all;

entity ra is
    port (
      p1 : in  logicsig;
      p2 : in  coaxsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  coaxsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end ra;
architecture gates of ra is
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

  component raslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  coaxsig;
      f : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p6,
    y => d);

  tp2 <= d;

  u2 : inv port map (
    a => p3,
    y => c);

  tp3 <= c;

  u3 : inv port map (
    a => p16,
    y => a);

  tp4 <= a;

  u4 : inv port map (
    a => p25,
    y => b);

  tp6 <= b;

  u5 : raslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p2,
    f => p15,
    tp => tp1,
    y => p4,
    y1 => p13,
    y2 => p11,
    y3 => p7,
    y4 => p9);


  u6 : raslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p26,
    f => p15,
    tp => tp5,
    y => p28,
    y1 => p23,
    y2 => p21,
    y3 => p19,
    y4 => p17);


  u7 : inv port map (
    a => p1,
    y => t1);


  u8 : g2 port map (
    a => a,
    b => t1,
    y => p8);


  u9 : g2 port map (
    a => b,
    b => t1,
    y => p10);


  u10 : g2 port map (
    a => c,
    b => t1,
    y => p14);


  u11 : g2 port map (
    a => d,
    b => t1,
    y => p12);


  u12 : inv port map (
    a => p27,
    y => t2);


  u13 : g2 port map (
    a => a,
    b => t2,
    y => p18);


  u14 : g2 port map (
    a => b,
    b => t2,
    y => p20);


  u15 : g2 port map (
    a => c,
    b => t2,
    y => p22);


  u16 : g2 port map (
    a => d,
    b => t2,
    y => p24);


  p5 <= '0';

end gates;

