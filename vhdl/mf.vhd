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
-- MF module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity mfslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      tp : out logicsig;
      q1_q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig;
      q6 : out logicsig);

end mfslice;
architecture gates of mfslice is
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
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;

begin -- gates
  u1 : inv port map (
    a => in1,
    y => t1);


  u2 : inv port map (
    a => in2,
    y => t2);


  u3 : g2 port map (
    a => in1,
    b => t2,
    y => t3);


  u4 : g2 port map (
    a => in2,
    b => t1,
    y => t4);


  u5 : g2 port map (
    a => t3,
    b => t4,
    y => tp,
    y2 => t5);

  q1_q2 <= t5;

  u6 : g2 port map (
    a => t5,
    b => a,
    y => q3);


  u7 : g2 port map (
    a => t5,
    b => b,
    y => q4);


  u8 : g2 port map (
    a => t5,
    b => c,
    y => q5);


  u9 : g2 port map (
    a => t5,
    b => d,
    y => q6);



end gates;

use work.sigs.all;

entity mf is
    port (
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p15 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      p1 : out logicsig;
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5_p6 : out logicsig;
      p11_p13 : out logicsig;
      p12 : out logicsig;
      p14 : out logicsig;
      p16 : out logicsig;
      p18 : out logicsig;
      p23_p24 : out logicsig;
      p25 : out logicsig;
      p26 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end mf;
architecture gates of mf is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component mfslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      tp : out logicsig;
      q1_q2 : out logicsig;
      q3 : out logicsig;
      q4 : out logicsig;
      q5 : out logicsig;
      q6 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv port map (
    a => p20,
    y => a);

  tp5 <= a;

  u2 : inv port map (
    a => p19,
    y => b);

  tp3 <= b;

  u3 : inv port map (
    a => p17,
    y => c);

  tp2 <= c;

  u4 : inv port map (
    a => p15,
    y => d);


  u5 : mfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p7,
    in2 => p8,
    q1_q2 => p5_p6,
    q3 => p3,
    q4 => p4,
    q5 => p2,
    q6 => p1,
    tp => tp1);


  u6 : mfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p9,
    in2 => p10,
    q1_q2 => p11_p13,
    q3 => p18,
    q4 => p16,
    q5 => p14,
    q6 => p12,
    tp => tp4);


  u7 : mfslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    in1 => p22,
    in2 => p21,
    q1_q2 => p23_p24,
    q3 => p25,
    q4 => p26,
    q5 => p27,
    q6 => p28);



end gates;

