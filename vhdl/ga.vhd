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
-- GA module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity gaslice is
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      in1 : in  coaxsig;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

end gaslice;
architecture gates of gaslice is
  component cxreceiver
    port (
      a : in  coaxsig;
      y : out std_logic);

  end component;

  component g2
    port (
      a : in  std_logic;
      b : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  component rsflop
    port (
      r : in  std_logic;
      r2 : in  std_logic := '1';
      r3 : in  std_logic := '1';
      r4 : in  std_logic := '1';
      s : in  std_logic;
      s2 : in  std_logic := '1';
      s3 : in  std_logic := '1';
      s4 : in  std_logic := '1';
      q : out std_logic;
      qb : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;
  signal t3 : std_logic;
  signal t4 : std_logic;
  signal t5 : std_logic;
  signal t6 : std_logic;

begin -- gates
  u1 : cxreceiver port map (
    a => in1,
    y => t1);


  u2 : rsflop port map (
    r => a,
    s => t1,
    q => t2,
    qb => t3);

  q1 <= t2;
  tp <= t2;

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

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ga is
    port (
      p2 : in  std_logic;
      p9 : in  std_logic;
      p11 : in  std_logic;
      p12 : in  coaxsig;
      p14 : in  coaxsig;
      p16 : in  coaxsig;
      p18 : in  coaxsig;
      p28 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p3 : out std_logic;
      p5 : out std_logic;
      p7 : out std_logic;
      p17 : out std_logic;
      p19 : out std_logic;
      p20 : out std_logic;
      p21 : out std_logic;
      p22 : out std_logic;
      p24 : out std_logic;
      p25 : out std_logic;
      p26 : out std_logic;
      p27 : out std_logic);

end ga;
architecture gates of ga is
  component gaslice
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      in1 : in  coaxsig;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

  end component;

  component inv
    port (
      a : in  std_logic;
      y : out std_logic);

  end component;

  component inv2
    port (
      a : in  std_logic;
      y : out std_logic;
      y2 : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;

begin -- gates
  u1 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p12,
    q1 => p20,
    q2 => p25,
    q3 => p1,
    tp => tp1);


  u2 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p14,
    q1 => p22,
    q2 => p21,
    q3 => p3,
    tp => tp2);


  u3 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p16,
    q1 => p24,
    q2 => p19,
    q3 => p5,
    tp => tp5);


  u4 : gaslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    in1 => p18,
    q1 => p26,
    q2 => p17,
    q3 => p7,
    tp => tp6);


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

