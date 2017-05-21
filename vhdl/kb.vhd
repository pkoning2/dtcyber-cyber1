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
-- KB module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kbslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      tp : out logicsig;
      y : out logicsig;
      y2 : out logicsig);

end kbslice;
architecture gates of kbslice is
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
    s => c,
    s2 => d,
    q => t1);

  tp <= t1;

  u2 : inv port map (
    a => t1,
    y => y);


  u3 : g2 port map (
    a => t1,
    b => b,
    y => y2);



end gates;

use work.sigs.all;

entity kb is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3_p5 : out logicsig;
      p7 : out logicsig;
      p8_p6_p12 : out logicsig;
      p10 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p20 : out logicsig;
      p21_p19_p17 : out logicsig;
      p25 : out logicsig;
      p28_p26_p22 : out logicsig);

end kb;
architecture gates of kb is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component kbslice
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


  u3 : kbslice port map (
    a => a,
    b => b,
    c => p9,
    d => p11,
    tp => tp1,
    y => p8_p6_p12,
    y2 => p10);


  u4 : kbslice port map (
    a => a,
    b => b,
    c => p2,
    d => p4,
    tp => tp2,
    y => p1_p3_p5,
    y2 => p7);


  u5 : kbslice port map (
    a => a,
    b => b,
    c => p24,
    d => p27,
    tp => tp5,
    y => p28_p26_p22,
    y2 => p20);


  u6 : kbslice port map (
    a => a,
    b => b,
    c => p18,
    d => p23,
    tp => tp6,
    y => p21_p19_p17,
    y2 => p15);


  p25 <= '0';

end gates;

