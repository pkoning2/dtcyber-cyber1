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
-- QD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qdslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      ca : in  logicsig;
      cb : in  logicsig;
      cc : in  logicsig;
      cd : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      y : out logicsig);

end qdslice;
architecture gates of qdslice is
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

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => cc,
    b => a,
    y => t1);


  u2 : g2 port map (
    a => t1,
    b => b,
    y => t2,
    y2 => t3);

  tp2 <= t3;

  u3 : g3 port map (
    a => t2,
    b => c,
    c => ca,
    y => t4);


  u4 : g3 port map (
    a => t3,
    b => d,
    c => ca,
    y => t5);


  u5 : g2 port map (
    a => t3,
    b => cb,
    y => t6);


  u6 : g4 port map (
    a => cd,
    b => e,
    c => f,
    d => g,
    y => t7);


  u7 : g4 port map (
    a => t7,
    b => t6,
    c => t4,
    d => t5,
    y => t8);

  tp1 <= t8;
  y <= t8;


end gates;

use work.sigs.all;

entity qd is
    port (
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p13 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p15 : out logicsig;
      p16 : out logicsig;
      p19 : out logicsig);

end qd;
architecture gates of qd is
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

  component qdslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      ca : in  logicsig;
      cb : in  logicsig;
      cc : in  logicsig;
      cd : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p8,
    y => a);


  u2 : inv port map (
    a => p18,
    y => b);


  u3 : inv port map (
    a => p23,
    y => c);


  u4 : inv port map (
    a => p13,
    y => d);


  u5 : inv2 port map (
    a => p21,
    y2 => t1);

  p10 <= t1;
  p15 <= t1;
  p16 <= t1;

  u6 : qdslice port map (
    a => p11,
    b => p4,
    c => p9,
    ca => a,
    cb => b,
    cc => c,
    cd => d,
    d => p6,
    e => p7,
    f => p3,
    g => p5,
    tp1 => tp1,
    tp2 => tp2,
    y => p12);


  u7 : qdslice port map (
    a => p20,
    b => p27,
    c => p22,
    ca => a,
    cb => b,
    cc => c,
    cd => d,
    d => p25,
    e => p24,
    f => p26,
    g => p28,
    tp1 => tp6,
    tp2 => tp5,
    y => p19);



end gates;

