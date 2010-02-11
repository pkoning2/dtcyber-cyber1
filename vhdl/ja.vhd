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
-- JA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jaslice is
    port (
      a : in  logicsig;
      c : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      i3 : in  coaxsig;
      i4 : in  logicsig;
      tp1_y1 : out logicsig;
      tp2 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end jaslice;
architecture gates of jaslice is
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

  component rs4flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

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
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => i1,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => t2);

  tp1_y1 <= t2;

  u3 : inv port map (
    a => t2,
    y => t3);


  u4 : g2 port map (
    a => c,
    b => i2,
    y => t4);


  u5 : cxreceiver port map (
    a => i3,
    y => t5);


  u6 : g2 port map (
    a => i4,
    b => e,
    y => t6);


  u7 : rs4flop port map (
    r => a,
    s => t4,
    s2 => t5,
    s3 => t6,
    s4 => '1',
    q => t7);

  tp2 <= t7;

  u8 : inv port map (
    a => t7,
    y => t8);


  u9 : g2 port map (
    a => t7,
    b => f,
    y => t9);


  u10 : g2 port map (
    a => t8,
    b => g,
    y => t10);


  u11 : g2 port map (
    a => t9,
    b => t10,
    y => t11,
    y2 => y3);


  u12 : g2 port map (
    a => t3,
    b => t11,
    y => y2);



end gates;

use work.sigs.all;

entity ja is
    port (
      p3 : in  coaxsig;
      p5 : in  logicsig;
      p7 : in  coaxsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  coaxsig;
      p25 : in  logicsig;
      p27 : in  coaxsig;
      tp1 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p11_tp2 : out logicsig;
      p13 : out logicsig;
      p18 : out logicsig;
      p20_tp5 : out logicsig;
      p28 : out logicsig);

end ja;
architecture gates of ja is
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

  component jaslice
    port (
      a : in  logicsig;
      c : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      i3 : in  coaxsig;
      i4 : in  logicsig;
      tp1_y1 : out logicsig;
      tp2 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;

begin -- gates
  u1 : inv port map (
    a => p21,
    y => a);


  u2 : inv2 port map (
    a => p10,
    y2 => c);


  u3 : inv2 port map (
    a => p14,
    y2 => e);


  u4 : inv2 port map (
    a => p17,
    y => g,
    y2 => f);


  u5 : jaslice port map (
    a => a,
    c => c,
    e => e,
    f => f,
    g => g,
    i1 => p7,
    i2 => p5,
    i3 => p3,
    i4 => p8,
    tp1_y1 => p11_tp2,
    tp2 => tp1,
    y2 => p13,
    y3 => p1);


  u6 : jaslice port map (
    a => a,
    c => c,
    e => e,
    f => f,
    g => g,
    i1 => p23,
    i2 => p22,
    i3 => p27,
    i4 => p25,
    tp1_y1 => p20_tp5,
    tp2 => tp6,
    y2 => p18,
    y3 => p28);



end gates;

