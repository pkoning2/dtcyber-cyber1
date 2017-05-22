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
-- CS module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity csslice is
    port (
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      h : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig;
      y5 : out logicsig;
      y6 : out logicsig);

end csslice;
architecture gates of csslice is
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

begin -- gates
  u1 : cxreceiver port map (
    a => i1,
    y => t1);


  u2 : rsflop port map (
    r => c,
    s => t1,
    q => t2,
    qb => t3);


  u3 : g2 port map (
    a => t2,
    b => f,
    y => y1);


  u4 : g2 port map (
    a => t2,
    b => b,
    y => y2);


  u5 : g2 port map (
    a => t2,
    b => e,
    y => y3);


  u6 : g3 port map (
    a => t2,
    b => h,
    c => d,
    y => t4);


  u7 : g3 port map (
    a => t3,
    b => g,
    c => d,
    y => t5);


  u8 : rs4flop port map (
    r => i4,
    s => i2,
    s2 => i3,
    s3 => t4,
    s4 => t5,
    q => y4,
    qb => t6);

  y6 <= t6;

  u9 : inv port map (
    a => t6,
    y => y5);



end gates;

use work.sigs.all;

entity cs is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  coaxsig;
      p15 : in  logicsig;
      p18 : in  coaxsig;
      p22 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p9 : out logicsig;
      p10_tp1 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p23_tp6 : out logicsig;
      p25_tp5 : out logicsig;
      p27 : out logicsig);

end cs;
architecture gates of cs is
  component csslice
    port (
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      h : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig;
      y4 : out logicsig;
      y5 : out logicsig;
      y6 : out logicsig);

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

  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;

begin -- gates
  u1 : inv port map (
    a => p22,
    y => b);


  u2 : inv port map (
    a => p12,
    y => c);


  u3 : inv port map (
    a => p15,
    y => d);


  u4 : inv port map (
    a => p28,
    y => e);


  u5 : inv port map (
    a => p26,
    y => f);


  u6 : inv2 port map (
    a => p13,
    y => g,
    y2 => h);


  u7 : csslice port map (
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    g => g,
    h => h,
    i1 => p14,
    i2 => p8,
    i3 => p6,
    i4 => p11,
    y1 => p23_tp6,
    y2 => p21,
    y3 => p20,
    y4 => p4,
    y5 => p10_tp1,
    y6 => p9);


  u8 : csslice port map (
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    g => g,
    h => h,
    i1 => p18,
    i2 => p7,
    i3 => p5,
    i4 => p11,
    y1 => p25_tp5,
    y2 => p19,
    y3 => p27,
    y4 => p3,
    y5 => p1,
    y6 => p2);



end gates;

