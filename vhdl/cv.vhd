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
-- CV module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity cv is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p20 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig;
      p12 : out logicsig;
      p19 : out logicsig;
      p23 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end cv;
architecture gates of cv is
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

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
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
  signal t7 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p16,
    y => t1);


  u2 : latch port map (
    clk => p18,
    d => t1,
    q => a,
    qb => b);

  tp6 <= a;

  u3 : inv port map (
    a => p13,
    y => t2);


  u4 : latch port map (
    clk => p18,
    d => t2,
    q => c,
    qb => d);

  tp2 <= c;

  u5 : inv port map (
    a => p17,
    y => t3);


  u6 : latch port map (
    clk => p18,
    d => t3,
    q => e,
    qb => f);

  tp1 <= e;

  u7 : g2 port map (
    a => p2,
    b => p3,
    y => p1);


  u8 : g3 port map (
    a => b,
    b => d,
    c => f,
    y2 => p28);


  u9 : g3 port map (
    a => a,
    b => d,
    c => f,
    y2 => p23);


  u10 : g3 port map (
    a => b,
    b => c,
    c => f,
    y2 => p5);


  u11 : g3 port map (
    a => a,
    b => c,
    c => f,
    y2 => p6);


  u12 : g4 port map (
    a => b,
    b => d,
    c => e,
    d => p20,
    y2 => p19);


  u13 : inv port map (
    a => p20,
    y => t4);


  u14 : g4 port map (
    a => a,
    b => t4,
    c => d,
    d => e,
    y2 => p26);


  u15 : g4 port map (
    a => b,
    b => c,
    c => e,
    d => p9,
    y2 => p12);


  u16 : inv port map (
    a => p9,
    y => t5);


  u17 : g4 port map (
    a => a,
    b => t5,
    c => c,
    d => e,
    y2 => t7);



end gates;

