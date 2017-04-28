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
-- MR module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mrslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      y : out coaxsig);

end mrslice;
architecture gates of mrslice is
  component cxdriver5
    port (
      a : in  logicsig;
      a2 : in  logicsig;
      a3 : in  logicsig;
      a4 : in  logicsig;
      a5 : in  logicsig;
      y : out coaxsig);

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

  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => b,
    y => t3);


  u2 : g2 port map (
    a => c,
    b => d,
    y => t4);


  u3 : inv port map (
    a => d,
    y => t2);


  u4 : g2 port map (
    a => t2,
    b => e,
    y => t5);


  u5 : cxdriver5 port map (
    a => f,
    a2 => g,
    a3 => t3,
    a4 => t4,
    a5 => t5,
    y => y);



end gates;

use work.sigs.all;

entity mr is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      p1_tp1 : out coaxsig;
      p5_tp3 : out coaxsig;
      p25_tp4 : out coaxsig;
      p27_tp6 : out coaxsig);

end mr;
architecture gates of mr is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component mrslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      y : out coaxsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : inv port map (
    a => p19,
    y => a);


  u2 : inv port map (
    a => p21,
    y => b);


  u3 : inv port map (
    a => p9,
    y => c);


  u4 : mrslice port map (
    a => a,
    b => p2,
    c => b,
    d => p3,
    e => c,
    f => p14,
    g => p13,
    y => p1_tp1);


  u5 : mrslice port map (
    a => a,
    b => p8,
    c => b,
    d => p6,
    e => c,
    f => p12,
    g => p11,
    y => p5_tp3);


  u6 : mrslice port map (
    a => a,
    b => p24,
    c => b,
    d => p23,
    e => c,
    f => p18,
    g => p17,
    y => p25_tp4);


  u7 : mrslice port map (
    a => a,
    b => p28,
    c => b,
    d => p26,
    e => c,
    f => p16,
    g => p15,
    y => p27_tp6);



end gates;

