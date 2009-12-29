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
-- RV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rvslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

end rvslice;
architecture gates of rvslice is
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

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i,
    b => b,
    y => t1);


  u2 : rs2flop port map (
    r => a,
    s => t1,
    s2 => i2,
    q => t2);

  tp <= t2;

  u3 : inv2 port map (
    a => t2,
    y => q2,
    y2 => t3);

  q1 <= t3;
  q3 <= t3;


end gates;

use work.sigs.all;

entity rv is
    port (
      p6 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end rv;
architecture gates of rv is
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

  component rvslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      i : in  logicsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p28,
    y => c,
    y2 => d);


  u3 : rvslice port map (
    a => a,
    b => b,
    i => p13,
    i2 => p6,
    q1 => p8,
    q2 => p3,
    q3 => p10,
    tp => tp1);


  u4 : rvslice port map (
    a => a,
    b => b,
    i => p18,
    q1 => p19,
    q2 => p23,
    q3 => p5,
    tp => tp2);


  u5 : rvslice port map (
    a => a,
    b => b,
    i => p14,
    q1 => p7,
    q2 => p4,
    q3 => p9,
    tp => tp3);


  u6 : rvslice port map (
    a => c,
    b => d,
    i => p26,
    q1 => p25,
    q2 => p20,
    q3 => p27,
    tp => tp4);


  u7 : rvslice port map (
    a => c,
    b => d,
    i => p15,
    q1 => p11,
    q2 => p1,
    q3 => p2,
    tp => tp5);


  u8 : rvslice port map (
    a => c,
    b => d,
    i => p17,
    q2 => t2,
    q3 => p24,
    tp => tp6);

  p21 <= t2;

  u9 : g2 port map (
    a => p12,
    b => t2,
    y => p22);



end gates;

