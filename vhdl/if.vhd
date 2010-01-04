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
-- IF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity if is
    port (
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p10 : out logicsig;
      p12 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p15 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p20 : out logicsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end if;
architecture gates of if is
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

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;
  signal g : logicsig;
  signal h : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => e,
    y => t1);

  p17 <= t1;
  tp2 <= t1;

  u2 : inv port map (
    a => p7,
    y => e);


  u3 : inv port map (
    a => p26,
    y => a);


  u4 : inv port map (
    a => p23,
    y => f);


  u5 : inv port map (
    a => p8,
    y => b);


  u6 : inv port map (
    a => p6,
    y => g);

  tp1 <= g;

  u7 : inv port map (
    a => p11,
    y => c);


  u8 : inv port map (
    a => p24,
    y => h);


  u9 : inv port map (
    a => p9,
    y => d);


  u10 : g2 port map (
    a => e,
    b => b,
    y2 => p1);


  u11 : g2 port map (
    a => e,
    b => c,
    y2 => p3);


  u12 : g2 port map (
    a => e,
    b => d,
    y2 => p5);


  u13 : g2 port map (
    a => f,
    b => a,
    y2 => p27);


  u14 : g2 port map (
    a => f,
    b => b,
    y2 => p15);


  u15 : g2 port map (
    a => f,
    b => c,
    y => tp6,
    y2 => p25);


  u16 : g2 port map (
    a => f,
    b => d,
    y2 => p13);


  u17 : g2 port map (
    a => g,
    b => b,
    y2 => p10);


  u18 : g2 port map (
    a => g,
    b => c,
    y2 => p12);


  u19 : g2 port map (
    a => g,
    b => d,
    y2 => p14);


  u20 : g2 port map (
    a => h,
    b => a,
    y2 => p22);


  u21 : g2 port map (
    a => h,
    b => b,
    y2 => p20);


  u22 : g2 port map (
    a => h,
    b => c,
    y => tp5,
    y2 => p21);


  u23 : g2 port map (
    a => h,
    b => d,
    y2 => p18);



end gates;

