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
-- GA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity gaslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      in1 : in  coaxsig;
      q1_tp : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

end gaslice;
architecture gates of gaslice is
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
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

begin -- gates
  u1 : cxreceiver port map (
    a => in1,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => t2,
    qb => t3);

  q1_tp <= t2;

  u3 : g2 port map (
    a => b,
    b => t2,
    y => t4);


  u4 : g2 port map (
    a => t3,
    b => c,
    y => t5);


  u5 : g2 port map (
    a => t4,
    b => t5,
    y => t6);


  u6 : g2 port map (
    a => d,
    b => t6,
    y => q2);


  u7 : g2 port map (
    a => t6,
    b => e,
    y => q3);



end gates;

use work.sigs.all;

entity ga is
    port (
      p2 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  coaxsig;
      p14 : in  coaxsig;
      p16 : in  coaxsig;
      p18 : in  coaxsig;
      p28 : in  logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p17 : out logicsig;
      p19 : out logicsig;
      p20_tp1 : out logicsig;
      p21 : out logicsig;
      p22_tp2 : out logicsig;
      p24_tp5 : out logicsig;
      p25 : out logicsig;
      p26_tp6 : out logicsig;
      p27 : out logicsig);

end ga;
architecture gates of ga is
  component gaslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      in1 : in  coaxsig;
      q1_tp : out logicsig;
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
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;

begin -- gates
  u1 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p12,
    q1_tp => p20_tp1,
    q2 => p25,
    q3 => p1);


  u2 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p14,
    q1_tp => p22_tp2,
    q2 => p21,
    q3 => p3);


  u3 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p16,
    q1_tp => p24_tp5,
    q2 => p19,
    q3 => p5);


  u4 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p18,
    q1_tp => p26_tp6,
    q2 => p17,
    q3 => p7);


  u5 : inv port map (
    a => p9,
    y => a);


  u6 : inv2 port map (
    a => p11,
    y => b,
    y2 => c);


  u7 : inv port map (
    a => p28,
    y => d);


  u8 : inv port map (
    a => p2,
    y => e);


  p27 <= '0';

end gates;

