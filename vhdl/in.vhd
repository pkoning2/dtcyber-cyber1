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
-- IN module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mod_in is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p9 : out logicsig;
      p13 : out logicsig;
      p17 : out logicsig;
      p19_tp2 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end mod_in;
architecture gates of mod_in is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : rs4flop port map (
    r => p18,
    s => p16,
    s2 => p14,
    s3 => p12,
    s4 => '1',
    q => b,
    qb => c);


  u3 : g4 port map (
    a => p16,
    b => p14,
    c => p12,
    d => c,
    y => a);

  tp5 <= a;

  u4 : inv2 port map (
    a => b,
    y2 => p19_tp2);


  u5 : rsflop port map (
    r => p15,
    s => p3,
    q => tp6,
    qb => p17);


  u6 : inv port map (
    a => p2,
    y => t2);


  u7 : g2 port map (
    a => t2,
    b => b,
    y => p1);


  u8 : inv port map (
    a => p4,
    y => t3);


  u9 : g2 port map (
    a => t3,
    b => b,
    y => p7);


  u10 : inv port map (
    a => p6,
    y => t4);


  u11 : g2 port map (
    a => t4,
    b => b,
    y => p5);


  u12 : inv port map (
    a => p8,
    y => t5);


  u13 : g2 port map (
    a => t5,
    b => a,
    y => p9);


  u14 : inv port map (
    a => p22,
    y => t6);


  u15 : g2 port map (
    a => t6,
    b => a,
    y => p21);


  u16 : inv port map (
    a => p24,
    y => t7);


  u17 : g2 port map (
    a => t7,
    b => a,
    y => p23);


  u18 : inv port map (
    a => p26,
    y => t8);


  u19 : g2 port map (
    a => t8,
    b => a,
    y => p25);


  u20 : inv port map (
    a => p28,
    y => t9);


  u21 : g2 port map (
    a => t9,
    b => a,
    y => p27);


  u22 : g3 port map (
    a => p20,
    b => p11,
    c => p10,
    y => p13);



end gates;

