-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2009-2010 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- KD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity kd is
    port (
      p3 : in  logicsig;
      p6 : in  logicsig;
      p25 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp3 : out logicsig;
      p1_tp2 : out logicsig;
      p8_tp5 : out logicsig;
      p23_tp6 : out logicsig);

end kd;
architecture gates of kd is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g6
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal g : logicsig;
  signal i : logicsig;
  signal j : logicsig;
  signal k : logicsig;
  signal l : logicsig;
  signal m : logicsig;
  signal n : logicsig;
  signal o : logicsig;
  signal p : logicsig;
  signal q : logicsig;
  signal r : logicsig;
  signal s : logicsig;
  signal t : logicsig;
  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;
  signal t7 : logicsig;
  signal t8 : logicsig;
  signal t11 : logicsig;
  signal t12 : logicsig;
  signal t13 : logicsig;
  signal t14 : logicsig;
  signal t15 : logicsig;
  signal t16 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => c,
    b => p3,
    y => t1);


  u2 : inv port map (
    a => p3,
    y => d);


  u3 : g2 port map (
    a => d,
    b => p6,
    y => t2);


  u4 : inv port map (
    a => p6,
    y => c);


  u5 : g2 port map (
    a => t1,
    b => t2,
    y => g,
    y2 => t3);

  tp1 <= g;

  u6 : g2 port map (
    a => t3,
    b => e,
    y => t4);


  u7 : g2 port map (
    a => a,
    b => p25,
    y => t5);


  u8 : inv port map (
    a => p25,
    y => b);


  u9 : g2 port map (
    a => b,
    b => p28,
    y => t6);


  u10 : inv port map (
    a => p28,
    y => a);


  u11 : g2 port map (
    a => t5,
    b => t6,
    y => e,
    y2 => t7);


  u12 : g2 port map (
    a => g,
    b => t7,
    y => t8);


  u13 : g2 port map (
    a => t4,
    b => t8,
    y => p23_tp6);


  u14 : g2 port map (
    a => d,
    b => c,
    y => s,
    y2 => t);


  u15 : g2 port map (
    a => d,
    b => b,
    y => q,
    y2 => r);


  u16 : g2 port map (
    a => c,
    b => b,
    y => o,
    y2 => p);


  u17 : g2 port map (
    a => d,
    b => a,
    y => m,
    y2 => n);


  u18 : g2 port map (
    a => c,
    b => a,
    y => k,
    y2 => l);


  u19 : g2 port map (
    a => b,
    b => a,
    y => i,
    y2 => j);


  u20 : g2 port map (
    a => t,
    b => j,
    y2 => p1_tp2);


  u21 : g2 port map (
    a => j,
    b => s,
    y => t11);


  u22 : g2 port map (
    a => l,
    b => q,
    y => t12);


  u23 : g2 port map (
    a => n,
    b => o,
    y => t13);


  u24 : g2 port map (
    a => p,
    b => m,
    y => t14);

  tp3 <= t14;

  u25 : g2 port map (
    a => r,
    b => k,
    y => t15);


  u26 : g2 port map (
    a => t,
    b => i,
    y => t16);


  u27 : g6 port map (
    a => t11,
    b => t12,
    c => t13,
    d => t14,
    e => t15,
    f => t16,
    y => p8_tp5);



end gates;

