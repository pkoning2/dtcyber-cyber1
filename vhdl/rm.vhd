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

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity rmslice is
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      in3 : in  std_logic;
      in4 : in  std_logic;
      in5 : in  std_logic;
      tp : out coaxsig;
      y : out coaxsig);

end rmslice;
architecture gates of rmslice is
  component cxdriver5
    port (
      a : in  std_logic;
      a2 : in  std_logic;
      a3 : in  std_logic;
      a4 : in  std_logic;
      a5 : in  std_logic;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
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

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity rm is
    port (
      p2 : in  std_logic;
      p3 : in  std_logic;
      p4 : in  std_logic;
      p5 : in  std_logic;
      p7 : in  std_logic;
      p8 : in  std_logic;
      p9 : in  std_logic;
      p10 : in  std_logic;
      p11 : in  std_logic;
      p12 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p16 : in  std_logic;
      p17 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p20 : in  std_logic;
      p21 : in  std_logic;
      p22 : in  std_logic;
      p24 : in  std_logic;
      p25 : in  std_logic;
      p26 : in  std_logic;
      p27 : in  std_logic;
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
      a : in  std_logic;
      y : out std_logic);

  end component;

  component rmslice
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      in1 : in  std_logic;
      in2 : in  std_logic;
      in3 : in  std_logic;
      in4 : in  std_logic;
      in5 : in  std_logic;
      tp : out coaxsig;
      y : out coaxsig);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;

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

