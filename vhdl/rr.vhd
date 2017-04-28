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
-- RR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rrslice is
    port (
      a : in  coaxsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      h : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y2 : out logicsig);

end rrslice;
architecture gates of rrslice is
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

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => a,
    y => t1);


  u2 : rsflop port map (
    r => d,
    s => t1,
    q => t2);

  tp <= t2;

  u3 : g3 port map (
    a => t2,
    b => b,
    c => c,
    y => t4);


  u4 : g3 port map (
    a => t2,
    b => e,
    c => f,
    y => t5);


  u5 : g3 port map (
    a => t4,
    b => t5,
    c => g,
    y => t6);

  y <= t6;

  u6 : g2 port map (
    a => t6,
    b => h,
    y2 => y2);



end gates;

use work.sigs.all;

entity rr is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  coaxsig;
      p14 : in  coaxsig;
      p15 : in  coaxsig;
      p17 : in  coaxsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp5 : out logicsig;
      p1_p3 : out logicsig;
      p6 : out logicsig;
      p10_tp1 : out logicsig;
      p13 : out logicsig;
      p16 : out logicsig;
      p19_tp6 : out logicsig;
      p23 : out logicsig;
      p28_p26 : out logicsig);

end rr;
architecture gates of rr is
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

  component rrslice
    port (
      a : in  coaxsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      h : in  logicsig;
      tp : out logicsig;
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : rrslice port map (
    a => p12,
    b => b,
    c => d,
    d => a,
    e => c,
    f => e,
    g => p8,
    h => p5,
    tp => tp2,
    y => p10_tp1,
    y2 => p6);

  p16 <= e;
  tp3 <= b;

  u2 : rrslice port map (
    a => p17,
    b => g,
    c => k,
    d => f,
    e => h,
    f => l,
    g => p21,
    h => p24,
    tp => tp5,
    y => p19_tp6,
    y2 => p23);

  p13 <= l;

  u3 : cxreceiver port map (
    a => p14,
    y => t1);


  u4 : rsflop port map (
    r => a,
    s => t1,
    q => b,
    qb => c);


  u5 : inv2 port map (
    a => p7,
    y2 => a);


  u6 : g2 port map (
    a => p2,
    b => p4,
    y2 => p1_p3);


  u7 : inv2 port map (
    a => p9,
    y => d,
    y2 => e);


  u8 : cxreceiver port map (
    a => p15,
    y => t2);


  u9 : rsflop port map (
    r => f,
    s => t2,
    q => g,
    qb => h);


  u10 : inv2 port map (
    a => p22,
    y2 => f);


  u11 : g2 port map (
    a => p27,
    b => p25,
    y2 => p28_p26);


  u12 : inv2 port map (
    a => p20,
    y => k,
    y2 => l);



end gates;

