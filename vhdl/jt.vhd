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
-- JT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jt is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      p6_tp2 : out logicsig;
      p8 : out logicsig;
      p13_p16 : out logicsig;
      p14_p17 : out logicsig;
      p19 : out logicsig;
      p25_tp5 : out logicsig;
      p26_tp6 : out logicsig);

end jt;
architecture gates of jt is
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
  signal m : logicsig;
  signal n : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t6 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;

begin -- gates
  u1 : rsflop port map (
    r => p7,
    s => p12,
    q => a,
    qb => f);

  tp3 <= a;

  u2 : g2 port map (
    a => a,
    b => p20,
    y => p14_p17);


  u3 : rsflop port map (
    r => p7,
    s => p3,
    q => b,
    qb => g);


  u4 : g2 port map (
    a => b,
    b => p18,
    y => p13_p16);


  u5 : g2 port map (
    a => f,
    b => g,
    y => t3);


  u6 : g2 port map (
    a => t3,
    b => p23,
    y => c);

  tp4 <= c;

  u7 : g3 port map (
    a => f,
    b => g,
    c => p10,
    y => p8);


  u8 : rs2flop port map (
    r => c,
    s => p28,
    s2 => p27,
    q => m);


  u9 : g2 port map (
    a => m,
    b => d,
    y => t4);


  u10 : rsflop port map (
    r => e,
    s => t4,
    q => p25_tp5);


  u11 : g2 port map (
    a => m,
    b => n,
    y2 => p19);


  u12 : rs2flop port map (
    r => c,
    s => p24,
    s2 => p22,
    q => n);


  u13 : g2 port map (
    a => n,
    b => d,
    y => t6);


  u14 : rsflop port map (
    r => e,
    s => t6,
    q => p26_tp6);


  u15 : rs2flop port map (
    r => c,
    s => p1,
    s2 => p4,
    q => t8);


  u16 : g2 port map (
    a => t8,
    b => d,
    y => t9);


  u17 : rsflop port map (
    r => e,
    s => t9,
    q => p6_tp2);


  u18 : inv2 port map (
    a => p9,
    y2 => d);


  u19 : inv port map (
    a => p21,
    y => e);



end gates;

