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
-- RM module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rmslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      in3 : in  logicsig;
      in4 : in  logicsig;
      in5 : in  logicsig;
      tp : out coaxsig;
      y : out coaxsig);

end rmslice;
architecture gates of rmslice is
  component cxdriver5
    port (
      a : in  logicsig;
      a2 : in  logicsig;
      a3 : in  logicsig;
      a4 : in  logicsig;
      a5 : in  logicsig;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : coaxsig;

begin -- gates
  u1 : g2 port map (
    a => in1,
    b => a,
    y => t1);


  u2 : g2 port map (
    a => in2,
    b => b,
    y => t2);


  u3 : g2 port map (
    a => in3,
    b => c,
    y => t3);


  u4 : cxdriver5 port map (
    a => in4,
    a2 => in5,
    a3 => t1,
    a4 => t2,
    a5 => t3,
    y => t4);

  tp <= t4;
  y <= t4;


end gates;

use work.sigs.all;

entity rm is
    port (
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out coaxsig;
      tp2 : out coaxsig;
      tp5 : out coaxsig;
      tp6 : out coaxsig;
      p1 : out coaxsig;
      p6 : out coaxsig;
      p23 : out coaxsig;
      p28 : out coaxsig);

end rm;
architecture gates of rm is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component rmslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      in1 : in  logicsig;
      in2 : in  logicsig;
      in3 : in  logicsig;
      in4 : in  logicsig;
      in5 : in  logicsig;
      tp : out coaxsig;
      y : out coaxsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;

begin -- gates
  u1 : rmslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p7,
    in2 => p3,
    in3 => p5,
    in4 => p2,
    in5 => p4,
    tp => tp1,
    y => p1);


  u2 : rmslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p13,
    in2 => p9,
    in3 => p11,
    in4 => p8,
    in5 => p10,
    tp => tp2,
    y => p6);


  u3 : rmslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p20,
    in2 => p18,
    in3 => p16,
    in4 => p19,
    in5 => p21,
    tp => tp5,
    y => p23);


  u4 : rmslice port map (
    a => a,
    b => b,
    c => c,
    in1 => p22,
    in2 => p24,
    in3 => p26,
    in4 => p27,
    in5 => p25,
    tp => tp6,
    y => p28);


  u5 : inv port map (
    a => p12,
    y => a);


  u6 : inv port map (
    a => p14,
    y => b);


  u7 : inv port map (
    a => p17,
    y => c);



end gates;

