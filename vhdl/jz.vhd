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
-- JZ module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity jzslice is
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      a : out logicsig;
      q : out logicsig;
      qb1 : out logicsig;
      qb2 : out logicsig;
      qb3 : out logicsig);

end jzslice;
architecture gates of jzslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;

begin -- gates
  u1 : rs4flop port map (
    r => i4,
    s => i1,
    s2 => i2,
    s3 => i3,
    s4 => '1',
    q => t1);

  q <= t1;
  tp1 <= t1;

  u2 : inv2 port map (
    a => t1,
    y2 => a);


  u3 : rsflop port map (
    r => i4,
    s => i5,
    q => tp2,
    qb => t2);


  u4 : inv port map (
    a => t2,
    y => t3);

  qb1 <= t3;
  qb2 <= t3;
  qb3 <= t3;


end gates;

use work.sigs.all;

entity jzslice2 is
    port (
      a : in  logicsig;
      b : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      qb : out logicsig;
      y : out logicsig);

end jzslice2;
architecture gates of jzslice2 is
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
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : inv port map (
    a => i1,
    y => t1);


  u2 : inv port map (
    a => i2,
    y => t2);


  u3 : g2 port map (
    a => t1,
    b => a,
    y => t3);


  u4 : g2 port map (
    a => t2,
    b => b,
    y => t4);


  u5 : g2 port map (
    a => i3,
    b => i4,
    y => t5);

  y <= t5;

  u6 : rs2flop port map (
    r => t5,
    s => t3,
    s2 => t4,
    q => tp,
    qb => qb);



end gates;

use work.sigs.all;

entity jz is
    port (
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p10 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p15 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p9 : out logicsig;
      p11 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p17 : out logicsig;
      p23 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end jz;
architecture gates of jz is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component jzslice
    port (
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      i5 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      a : out logicsig;
      q : out logicsig;
      qb1 : out logicsig;
      qb2 : out logicsig;
      qb3 : out logicsig);

  end component;

  component jzslice2
    port (
      a : in  logicsig;
      b : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      qb : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : jzslice port map (
    i1 => p6,
    i2 => p7,
    i3 => p8,
    i4 => p4,
    i5 => p5,
    a => b,
    q => p9,
    qb1 => p2,
    qb2 => p3,
    qb3 => p1,
    tp1 => tp2,
    tp2 => tp1);


  u2 : jzslice port map (
    i1 => p20,
    i2 => p25,
    i3 => p21,
    i4 => p27,
    i5 => p22,
    a => a,
    q => p23,
    qb1 => p26,
    qb2 => p24,
    qb3 => p28,
    tp1 => tp5,
    tp2 => tp6);


  u3 : jzslice2 port map (
    a => a,
    b => b,
    i1 => p18,
    i2 => p10,
    i3 => t1,
    i4 => t2,
    qb => p11,
    tp => tp4,
    y => p16);


  u4 : inv port map (
    a => p12,
    y => t1);


  u5 : inv port map (
    a => p19,
    y => t2);


  u6 : jzslice2 port map (
    a => a,
    b => b,
    i1 => p15,
    i2 => p13,
    i3 => t1,
    i4 => p19,
    qb => p14,
    tp => tp3,
    y => p17);



end gates;

