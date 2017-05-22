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
-- KO module (variant of KB)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity koslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      tp : out logicsig;
      y1 : out logicsig;
      y2 : out logicsig;
      y3 : out logicsig);

end koslice;
architecture gates of koslice is
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : rs2flop port map (
    r => a,
    s => d,
    s2 => e,
    q => t1);

  tp <= t1;

  u2 : inv port map (
    a => t1,
    y => y2);


  u3 : g2 port map (
    a => t1,
    b => b,
    y => y1);


  u4 : g2 port map (
    a => t1,
    b => c,
    y => y3);



end gates;

use work.sigs.all;

entity ko is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p10_p12 : out logicsig;
      p14 : out logicsig;
      p15_p17 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p28_p26 : out logicsig);

end ko;
architecture gates of ko is
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

  component koslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;

begin -- gates
  u1 : inv port map (
    a => p16,
    y => a);

  p14 <= a;

  u2 : inv port map (
    a => p13,
    y => b);


  u7 : inv2 port map (
    a => p18,
    y2 => c);

  
  u3 : koslice port map (
    a => a,
    b => b,
    c => c,
    d => p9,
    e => p11,
    tp => tp2,
    y1 => p6
    y2 => p10_p12,
    y3 => p8);


  u4 : koslice port map (
    a => a,
    b => b,
    c => c,
    d => p2,
    e => p4,
    tp => tp1,
    y1 => p7
    y2 => p1_p3,
    y3 => p5);


  u5 : koslice port map (
    a => a,
    b => b,
    c => c,
    d => p25,
    e => p27,
    tp => tp6,
    y1 => p22,
    y2 => p28_p26,
    y3 => p24);


  u6 : koslice port map (
    a => a,
    b => b,
    c => c,
    d => p19,
    e => p20,
    tp => tp5,
    y1 => p23
    y2 => p15_p17,
    y3 => p21);

end gates;

