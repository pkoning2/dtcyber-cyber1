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
-- KT module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kt is
    port (
      p6 : in  coaxsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  coaxsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p17 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end kt;
architecture gates of kt is
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
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p6,
    y => t1);


  u2 : rs2flop port map (
    r => c,
    s => p18,
    s2 => t1,
    q => t2,
    qb => t3);

  tp1 <= t2;

  u3 : g2 port map (
    a => a,
    b => t2,
    y => t4);


  u4 : g2 port map (
    a => b,
    b => t3,
    y => t5);


  u5 : g2 port map (
    a => t4,
    b => t5,
    y => t6);


  u6 : g2 port map (
    a => f,
    b => t6,
    y => p4);


  u7 : g2 port map (
    a => t6,
    b => e,
    y => p3);


  u8 : g3 port map (
    a => t6,
    b => d,
    c => p19,
    y => p1);


  u9 : cxreceiver port map (
    a => p24,
    y => t7);


  u10 : rs2flop port map (
    r => c,
    s => p26,
    s2 => t7,
    q => t8,
    qb => t9);

  tp6 <= t8;

  u11 : g2 port map (
    a => a,
    b => t8,
    y => t10);


  u12 : g2 port map (
    a => b,
    b => t9,
    y => t11);


  u13 : g2 port map (
    a => t10,
    b => t11,
    y => t12);


  u14 : g3 port map (
    a => p19,
    b => d,
    c => t12,
    y => p23);


  u15 : g2 port map (
    a => t12,
    b => e,
    y => p25);


  u16 : g2 port map (
    a => t12,
    b => f,
    y => p28);


  u17 : rsflop port map (
    r => c,
    s => p7,
    q => t13);

  tp2 <= t13;

  u18 : g2 port map (
    a => f,
    b => t13,
    y => p12);


  u19 : g2 port map (
    a => t13,
    b => e,
    y => p10);


  u20 : g3 port map (
    a => t13,
    b => d,
    c => p9,
    y => p11);


  u21 : rsflop port map (
    r => c,
    s => p21,
    q => t14);

  tp5 <= t14;

  u22 : g3 port map (
    a => p9,
    b => d,
    c => t14,
    y => p17);


  u23 : g2 port map (
    a => t14,
    b => e,
    y => p22);


  u24 : g2 port map (
    a => t14,
    b => f,
    y => p20);


  u25 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u26 : inv port map (
    a => p15,
    y => c);


  u27 : inv port map (
    a => p18,
    y => d);


  u28 : inv port map (
    a => p14,
    y => e);


  u29 : inv port map (
    a => p13,
    y => f);


  p27 <= '0';

end gates;

