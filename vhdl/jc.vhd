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
-- JC module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jc is
    port (
      p3 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end jc;
architecture gates of jc is
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
  signal d : logicsig;
  signal e : logicsig;
  signal p : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t9 : logicsig;
  signal t10 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;

begin -- gates
  u1 : rsflop port map (
    r => a,
    s => p24,
    q => r);

  p4 <= a;
  tp5 <= r;

  u2 : inv port map (
    a => p25,
    y => t1);

  p23 <= t1;

  u3 : g2 port map (
    a => t1,
    b => e,
    y => t2);


  u4 : g2 port map (
    a => r,
    b => c,
    y => t3);


  u5 : g2 port map (
    a => p21,
    b => d,
    y => t4);


  u6 : rs4flop port map (
    r => b,
    s => t2,
    s2 => t3,
    s3 => t4,
    s4 => p26,
    q => p);

  tp6 <= p;

  u7 : inv2 port map (
    a => p,
    y => p27,
    y2 => t5);

  p17 <= t5;
  p28 <= t5;

  u8 : rsflop port map (
    r => a,
    s => p8,
    q => s);

  tp2 <= s;

  u9 : inv port map (
    a => p3,
    y => t9);

  p5 <= t9;

  u10 : g2 port map (
    a => t9,
    b => e,
    y => t10);


  u11 : g2 port map (
    a => s,
    b => c,
    y => t11);


  u12 : g2 port map (
    a => p7,
    b => d,
    y => t12);


  u13 : rs4flop port map (
    r => b,
    s => t10,
    s2 => t11,
    s3 => t12,
    s4 => p10,
    q => t);

  tp1 <= t;

  u14 : inv2 port map (
    a => t,
    y => p1,
    y2 => t13);

  p2 <= t13;
  p9 <= t13;

  u16 : inv2 port map (
    a => r,
    y => t15,
    y2 => p22);

  p19 <= t15;
  
  u17 : g2 port map (
    a => t15,
    b => p,
    y => p20);


  u18 : inv2 port map (
    a => s,
    y => t16,
    y2 => p12);

  p6 <= t16;

  u19 : g2 port map (
    a => t16,
    b => t,
    y => p14);


  u20 : inv port map (
    a => p13,
    y => a);


  u21 : inv port map (
    a => p18,
    y => b);


  u22 : inv port map (
    a => p15,
    y => c);


  u23 : inv port map (
    a => p16,
    y => d);


  u24 : inv port map (
    a => p11,
    y => e);



end gates;

