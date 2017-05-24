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
-- CT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ct is
    port (
      p8 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p20 : in  logicsig;
      p23 : in  coaxsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1_p3_p5_p7_p9_p11 : out logicsig;
      p2_p4_p6_p10_p12_p14 : out logicsig;
      p18_p19_p22 : out logicsig;
      p21 : out logicsig;
      p24 : out logicsig;
      p28 : out logicsig);

end ct;
architecture gates of ct is
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

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p8,
    y => a);


  u2 : inv2 port map (
    a => p16,
    y => c,
    y2 => b);


  u3 : inv port map (
    a => p15,
    y => d);


  u4 : inv port map (
    a => p26,
    y => e);


  u5 : inv port map (
    a => p27,
    y => f);


  u6 : inv port map (
    a => p20,
    y => g);


  u7 : inv port map (
    a => p25,
    y => h);


  u8 : cxreceiver port map (
    a => p23,
    y => t1);


  u9 : rsflop port map (
    r => h,
    s => t1,
    q => t2,
    qb => t3);

  tp6 <= t2;
  p24 <= t3;
         
  u10 : inv port map (
    a => t2,
    y => p18_p19_p22);


  u11 : inv port map (
    a => t3,
    y => t4);


  u12 : g2 port map (
    a => t4,
    b => e,
    y => p21);


  u13 : g2 port map (
    a => t4,
    b => f,
    y => p28);


  u14 : g2 port map (
    a => g,
    b => t2,
    y => t5);


  u15 : rsflop port map (
    r => p17,
    s => t5,
    q => t6,
    qb => t7);

  tp5 <= t6;

  u16 : g3 port map (
    a => c,
    b => t6,
    c => a,
    y => j);


  u17 : g3 port map (
    a => a,
    b => t7,
    c => b,
    y => k);


  u18 : g4 port map (
    a => j,
    b => k,
    c => l,
    d => p13,
    y => tp2,
    y2 => t8);

  p2_p4_p6_p10_p12_p14 <= t8;

  u19 : inv port map (
    a => t8,
    y => p1_p3_p5_p7_p9_p11);


  u20 : g2 port map (
    a => b,
    b => d,
    y => l);



end gates;

