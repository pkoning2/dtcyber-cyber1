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
-- KW module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kwslice is
    port (
      a : in  logicsig;
      x : in  logicsig;
      y : in  logicsig;
      b : out logicsig;
      g : out logicsig);

end kwslice;
architecture gates of kwslice is
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

  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => a,
    y => t1);


  u2 : g2 port map (
    a => y,
    b => t1,
    y => b);


  u4 : g2 port map (
    a => x,
    b => a,
    y => g);



end gates;

use work.sigs.all;

entity kw is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p25 : out logicsig;
      p28 : out logicsig);

end kw;
architecture gates of kw is
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

  component g5
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
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

  component kwslice
    port (
      a : in  logicsig;
      x : in  logicsig;
      y : in  logicsig;
      b : out logicsig;
      g : out logicsig);

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
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal l : logicsig;
  signal n : logicsig;
  signal r : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t9 : logicsig;
  signal x : logicsig;
  signal y : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p2,
    y => l,
    y2 => r);


  u2 : inv2 port map (
    a => p4,
    y => n,
    y2 => p1);


  u3 : inv2 port map (
    a => p6,
    y => t1,
    y2 => t2);


  u4 : g3 port map (
    a => l,
    b => t1,
    c => n,
    y2 => x);

  tp1 <= x;

  u5 : g3 port map (
    a => n,
    b => t2,
    c => r,
    y2 => y);


  u6 : kwslice port map (
    a => p11,
    x => x,
    y => y,
    b => a,
    g => f);


  u7 : kwslice port map (
    a => p5,
    x => x,
    y => y,
    b => b,
    g => g);


  u8 : kwslice port map (
    a => p10,
    x => x,
    y => y,
    b => c,
    g => h);


  u9 : kwslice port map (
    a => p7,
    x => x,
    y => y,
    b => d,
    g => i);


  u10 : kwslice port map (
    a => p9,
    x => x,
    y => y,
    b => e,
    g => j);


  u11 : g5 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    y => t3);

  tp6 <= t3;

  u12 : g5 port map (
    a => f,
    b => g,
    c => h,
    d => i,
    e => j,
    y => t4);

  tp5 <= t4;

  u13 : inv port map (
    a => p26,
    y => t5);


  u14 : g2 port map (
    a => t3,
    b => t5,
    y => t6);


  u15 : g2 port map (
    a => t5,
    b => t4,
    y => t7);


  u16 : inv port map (
    a => p27,
    y => t8);


  u17 : rs2flop port map (
    r => t8,
    s => t6,
    s2 => t7,
    q => t9);


  u18 : inv2 port map (
    a => t9,
    y => p28,
    y2 => p25);



end gates;

