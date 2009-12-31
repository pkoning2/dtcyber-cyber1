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
-- IK module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity ikslice is
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      n : in  logicsig;
      s : in  logicsig;
      t : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

end ikslice;
architecture gates of ikslice is
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
  u1 : g3 port map (
    a => n,
    b => i1,
    c => t,
    y => t1);


  u2 : rs2flop port map (
    r => s,
    s => i2,
    s2 => t1,
    q => t2);

  q1 <= t2;

  u3 : g2 port map (
    a => t2,
    b => a,
    y2 => t3);

  q2 <= t3;
  tp <= t3;

  u4 : inv port map (
    a => t3,
    y => q3);



end gates;

use work.sigs.all;

entity ik is
    port (
      p3 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p21 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ik;
architecture gates of ik is
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

  component ikslice
    port (
      a : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      n : in  logicsig;
      s : in  logicsig;
      t : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

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

  signal a : logicsig;
  signal n : logicsig;
  signal s : logicsig;
  signal t : logicsig;
  signal t1 : logicsig;
  signal x : logicsig;

begin -- gates
  u1 : ikslice port map (
    a => a,
    i1 => p6,
    i2 => p3,
    n => n,
    s => s,
    t => t,
    q1 => p4,
    q2 => p2,
    q3 => p1);

  tp2 <= t;
  tp5 <= a;

  u2 : ikslice port map (
    a => a,
    i1 => p7,
    i2 => p10,
    n => n,
    s => s,
    t => t,
    q1 => p9,
    q2 => p8,
    q3 => p5,
    tp => tp1);


  u3 : ikslice port map (
    a => a,
    i1 => p11,
    i2 => x,
    n => n,
    s => s,
    t => t,
    q1 => p12,
    q2 => p17,
    q3 => p15,
    tp => tp6);


  u4 : inv port map (
    a => p18,
    y => a);


  u5 : inv2 port map (
    a => p16,
    y => s,
    y2 => t);


  u6 : inv port map (
    a => p23,
    y => t1);


  u7 : g2 port map (
    a => p25,
    b => t1,
    y => p27);


  u8 : g2 port map (
    a => t1,
    b => p24,
    y => p28);


  u9 : g2 port map (
    a => p22,
    b => t1,
    y => p26);


  u10 : g3 port map (
    a => n,
    b => t,
    c => p20,
    y => p21);


  u11 : g3 port map (
    a => p20,
    b => t,
    c => p19,
    y => x);


  u12 : inv port map (
    a => p19,
    y => n);



end gates;

