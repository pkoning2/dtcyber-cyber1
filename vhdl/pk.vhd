-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- PK module rev D -- channel control
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pk is
    port (
      p5 : in  logicsig;
      p6 : in  logicsig;
      p8 : in  coaxsig := '1';
      p12 : in  logicsig;
      p14 : in  coaxsig := '1';
      p15 : in  coaxsig := '1';
      p17 : in  logicsig;
      p19 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  coaxsig := '1';
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out coaxsig;
      p9 : out coaxsig;
      p10 : out logicsig;
      p13 : out coaxsig;
      p18 : out coaxsig;
      p21 : out logicsig;
      p22 : out coaxsig;
      p24_p26_p28 : out logicsig);

end pk;
architecture gates of pk is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

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

  component r4s4flop
    port (
      r : in  logicsig;
      r2 : in  logicsig;
      r3 : in  logicsig;
      r4 : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      s3 : in  logicsig;
      s4 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal ai : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal ei : logicsig;
  signal fi : logicsig;
  signal fq : logicsig;
  signal ii : logicsig;
  signal p8i : logicsig;
  signal p14i : logicsig;
  signal p15i : logicsig;
  signal p25i : logicsig;

begin -- gates
  u1 : inv port map (
    a => p19,
    y => c);

  tp6 <= c;

  u2 : g2 port map (
    a => p6,
    b => c,
    y => fi);


  u3 : g2 port map (
    a => c,
    b => p17,
    y => ei);


  u4 : g2 port map (
    a => p5,
    b => c,
    y => b);

  tp1 <= b;

  u5 : g2 port map (
    a => p12,
    b => c,
    y => ai);


  u6 : g2 port map (
    a => p27,
    b => c,
    y => ii);


  u7 : cxdriver port map (
    a => fi,
    y => p13);


  u8 : cxdriver port map (
    a => ei,
    y => p9);


  u9 : cxdriver port map (
    a => b,
    y => p3);


  u10 : cxdriver port map (
    a => ai,
    y => p22);


  u11 : cxdriver port map (
    a => ii,
    y => p18);


  u12 : cxreceiver port map (
    a => p15,
    y => p15i);


  u13 : cxreceiver port map (
    a => p8,
    y => p8i);


  u14 : r4s4flop port map (
    r => ei,
    r2 => p8i,
    r3 => a,
    r4 => p23,
    s => fi,
    s2 => p15i,
    s3 => b,
    s4 => '1',
    q => fq,
    qb => p10);

  tp2 <= fq;
  tp5 <= a;

  u15 : inv port map (
    a => fq,
    y => p24_p26_p28);


  u18 : cxreceiver port map (
    a => p25,
    y => p25i);


  u19 : cxreceiver port map (
    a => p14,
    y => p14i);


  u20 : r4s4flop port map (
    r => ii,
    r2 => p14i,
    r3 => '1',
    r4 => '1',
    s => ai,
    s2 => p23,
    s3 => b,
    s4 => p25i,
    q => a,
    qb => p21);



end gates;

