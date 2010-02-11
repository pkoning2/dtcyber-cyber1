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
-- MD module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mdslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig := '1';
      in3 : in  logicsig := '1';
      q_tp : out logicsig;
      qb : out logicsig);

end mdslice;
architecture gates of mdslice is
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

begin -- gates
  u1 : g3 port map (
    a => in1,
    b => in2,
    c => a,
    y => t1);


  u2 : rs2flop port map (
    r => b,
    s => t1,
    s2 => in3,
    q => t2);

  q_tp <= t2;

  u3 : g2 port map (
    a => t2,
    b => c,
    y => qb);



end gates;

use work.sigs.all;

entity md is
    port (
      p4 : in  logicsig;
      p6 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      p2 : out logicsig;
      p3_tp1 : out logicsig;
      p5 : out logicsig;
      p7_tp2 : out logicsig;
      p8 : out logicsig;
      p9 : out logicsig;
      p10_tp3 : out logicsig;
      p15_tp5 : out logicsig;
      p19_tp4 : out logicsig;
      p20 : out logicsig;
      p23 : out logicsig;
      p25_tp6 : out logicsig);

end md;
architecture gates of md is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component mdslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig := '1';
      in3 : in  logicsig := '1';
      q_tp : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p4,
    q_tp => p3_tp1,
    qb => p2);


  u2 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p6,
    q_tp => p7_tp2,
    qb => p5);


  u3 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p12,
    q_tp => p10_tp3,
    qb => p8);


  u4 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p22,
    in2 => p24,
    in3 => p21,
    q_tp => p19_tp4,
    qb => p20);


  u5 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p16,
    in2 => p18,
    in3 => p17,
    q_tp => p15_tp5,
    qb => p9);


  u6 : mdslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p26,
    in2 => p28,
    in3 => p27,
    q_tp => p25_tp6,
    qb => p23);


  u7 : inv port map (
    a => p11,
    y => a);


  u8 : inv port map (
    a => p13,
    y => b);


  u9 : inv port map (
    a => p14,
    y => c);



end gates;

