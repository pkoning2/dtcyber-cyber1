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
-- MP module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mpslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig);

end mpslice;
architecture gates of mpslice is
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

begin -- gates
  u1 : g3 port map (
    a => b,
    b => c,
    c => d,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => y);



end gates;

use work.sigs.all;

entity mp is
    port (
      p2 : in  logicsig;
      p3 : in  coaxsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  coaxsig;
      p26 : in  logicsig;
      p27 : in  coaxsig;
      tp5 : out logicsig;
      p1_tp1 : out logicsig;
      p4 : out logicsig;
      p8_tp3 : out logicsig;
      p10_tp2 : out logicsig;
      p11_tp4 : out logicsig;
      p15 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p24 : out logicsig;
      p25_tp6 : out logicsig;
      p28 : out logicsig);

end mp;
architecture gates of mp is
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

  component mpslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
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

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => p3,
    y => t1);


  u2 : rs2flop port map (
    r => p2,
    s => t1,
    s2 => p5,
    q => p1_tp1,
    qb => p4);


  u3 : cxreceiver port map (
    a => p23,
    y => t2);


  u4 : cxreceiver port map (
    a => p27,
    y => t3);


  u5 : rs2flop port map (
    r => p22,
    s => t2,
    s2 => t3,
    q => p25_tp6,
    qb => p24);


  u6 : mpslice port map (
    a => a,
    b => b,
    c => p6,
    d => p7,
    y => p10_tp2);

  tp5 <= b;

  u7 : mpslice port map (
    a => a,
    b => b,
    c => p16,
    d => p17,
    y => p15);


  u8 : mpslice port map (
    a => a,
    b => b,
    c => p7,
    d => p9,
    y => p8_tp3);


  u9 : mpslice port map (
    a => a,
    b => b,
    c => p17,
    d => p20,
    y => p18);


  u10 : mpslice port map (
    a => a,
    b => b,
    c => p9,
    d => p12,
    y => p11_tp4);


  u11 : mpslice port map (
    a => a,
    b => b,
    c => p20,
    d => p21,
    y => p19);


  u12 : inv2 port map (
    a => p13,
    y => a,
    y2 => t4);


  u13 : g2 port map (
    a => t4,
    b => p14,
    y2 => b);


  u14 : inv port map (
    a => p26,
    y => p28);



end gates;

