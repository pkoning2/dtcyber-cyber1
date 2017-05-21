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
-- KM module (somewhat like KP)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity km is
    port (
      p7 : in  coaxsig;
      p8 : in  coaxsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  coaxsig;
      p24 : in  coaxsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end km;
architecture gates of km is
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
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t30 : logicsig;
  signal t31 : logicsig;
  signal t32 : logicsig;
  signal t33 : logicsig;
  signal t34 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p8,
    y => t1);


  u2 : rsflop port map (
    r => c,
    s => t1,
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
    a => t6,
    b => f,
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
    y => t10);


  u10 : rsflop port map (
    r => c,
    s => t10,
    q => t11,
    qb => t12);

  tp6 <= t11;

  u11 : g2 port map (
    a => t11,
    b => a,
    y => t13);


  u12 : g2 port map (
    a => t12,
    b => b,
    y => t14);


  u13 : g2 port map (
    a => t13,
    b => t14,
    y => t15);


  u14 : g3 port map (
    a => t15,
    b => d,
    c => p19,
    y => p23);


  u15 : g2 port map (
    a => t15,
    b => e,
    y => p25);


  u16 : g2 port map (
    a => t15,
    b => f,
    y => p28);


  u22 : cxreceiver port map (
    a => p7,
    y => t30);


  u23 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u24 : inv port map (
    a => p15,
    y => c);


  u25 : inv port map (
    a => p18,
    y => d);


  u26 : inv port map (
    a => p14,
    y => e);


  u27 : inv port map (
    a => p13,
    y => f);


  u28 : rsflop port map (
    r => c,
    s => t30,
    q => t31,
    qb => t32);

  tp2 <= t31;

  u29 : inv port map (
    a => t31,
    y => p6);


  u30 : g2 port map (
    a => f,
    b => t31,
    y => p12);


  u31 : g2 port map (
    a => e,
    b => t31,
    y => p10);


  u32 : g3 port map (
    a => t31,
    b => d,
    c => p9,
    y => p11);


  u33 : cxreceiver port map (
    a => p21,
    y => t33);


  u34 : rsflop port map (
    r => c,
    s => t33,
    q => t34);

  tp5 <= t34;

  u35 : g3 port map (
    a => p9,
    b => d,
    c => t34,
    y => p17);


  u36 : g2 port map (
    a => t34,
    b => e,
    y => p22);


  u37 : inv port map (
    a => t34,
    y => p26);


  u38 : g2 port map (
    a => t34,
    b => f,
    y => p20);


  p27 <= '0';

end gates;

