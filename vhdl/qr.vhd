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
-- QR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qrslice is
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
      tp1 : out logicsig;
      tp2 : out logicsig;
      h : out logicsig;
      i : out logicsig;
      l : out logicsig;
      m : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end qrslice;
architecture gates of qrslice is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

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
    q => t2,
    qb => i);

  tp1 <= t2;
  y1 <= t2;

  u3 : inv2 port map (
    a => t2,
    y => t3,
    y2 => h);


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
    q => t7,
    qb => m);

  l <= t7;
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

entity qr is
    port (
      p3 : in  coaxsig;
      p5 : in  logicsig;
      p7 : in  coaxsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p17 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  coaxsig;
      p25 : in  coaxsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p28 : out logicsig);

end qr;
architecture gates of qr is
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

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component qrslice
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
      tp1 : out logicsig;
      tp2 : out logicsig;
      h : out logicsig;
      i : out logicsig;
      l : out logicsig;
      m : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal l : logicsig;
  signal m : logicsig;

begin -- gates
  u1 : inv port map (
    a => p12,
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


  u5 : qrslice port map (
    a => a,
    c => c,
    e => e,
    f => f,
    g => g,
    i1 => p7,
    i2 => p5,
    i3 => p3,
    i4 => p8,
    tp1 => tp2,
    tp2 => tp1,
    y1 => p11,
    y2 => p9,
    y3 => p1);


  u6 : qrslice port map (
    a => a,
    c => c,
    e => e,
    f => f,
    g => g,
    i1 => p24,
    i2 => p22,
    i3 => p25,
    i4 => p23,
    h => h,
    i => i,
    l => l,
    m => m,
    tp1 => tp5,
    tp2 => tp6,
    y1 => p20,
    y2 => p18,
    y3 => p28);


  u7 : g2 port map (
    a => h,
    b => m,
    y2 => p19);


  u8 : g2 port map (
    a => i,
    b => l,
    y2 => p16);



end gates;

