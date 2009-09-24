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
-- KA module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity kaslice1 is
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      i1 : in  coaxsig;
      i2 : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

end kaslice1;
architecture gates of kaslice1 is
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

  component rs2flop
    port (
      r : in  std_logic;
      s : in  std_logic;
      s2 : in  std_logic;
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
    a => i1,
    y => t1);


  u2 : rs2flop port map (
    r => c,
    s => t1,
    s2 => i2,
    q => t2,
    qb => t3);

  tp <= t2;

  u3 : g2 port map (
    a => a,
    b => t2,
    y => t4);


  u4 : g2 port map (
    a => t3,
    b => b,
    y => t5);


  u5 : g2 port map (
    a => t4,
    b => t5,
    y => t6);


  u6 : g2 port map (
    a => t6,
    b => d,
    y => q1);


  u7 : g2 port map (
    a => t6,
    b => e,
    y => q2);


  u8 : g2 port map (
    a => t6,
    b => f,
    y => q3);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity kaslice2 is
    port (
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      i1 : in  coaxsig;
      i2 : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

end kaslice2;
architecture gates of kaslice2 is
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

  component rs2flop
    port (
      r : in  std_logic;
      s : in  std_logic;
      s2 : in  std_logic;
      q : out std_logic;
      qb : out std_logic);

  end component;

  signal t1 : std_logic;
  signal t2 : std_logic;

begin -- gates
  u1 : cxreceiver port map (
    a => i1,
    y => t1);


  u2 : rs2flop port map (
    r => c,
    s => t1,
    s2 => i2,
    q => t2);

  tp <= t2;

  u3 : g2 port map (
    a => t2,
    b => d,
    y => q1);


  u4 : g2 port map (
    a => t2,
    b => e,
    y => q2);


  u5 : g2 port map (
    a => t2,
    b => f,
    y => q3);



end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ka is
    port (
      p6 : in  coaxsig;
      p7 : in  coaxsig;
      p8 : in  std_logic;
      p9 : in  std_logic;
      p13 : in  std_logic;
      p14 : in  std_logic;
      p15 : in  std_logic;
      p16 : in  std_logic;
      p18 : in  std_logic;
      p19 : in  std_logic;
      p21 : in  coaxsig;
      p24 : in  coaxsig;
      p26 : in  std_logic;
      tp1 : out std_logic;
      tp2 : out std_logic;
      tp5 : out std_logic;
      tp6 : out std_logic;
      p1 : out std_logic;
      p3 : out std_logic;
      p4 : out std_logic;
      p10 : out std_logic;
      p11 : out std_logic;
      p12 : out std_logic;
      p17 : out std_logic;
      p20 : out std_logic;
      p22 : out std_logic;
      p23 : out std_logic;
      p25 : out std_logic;
      p27 : out std_logic;
      p28 : out std_logic);

end ka;
architecture gates of ka is
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

  component kaslice1
    port (
      a : in  std_logic;
      b : in  std_logic;
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      i1 : in  coaxsig;
      i2 : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

  end component;

  component kaslice2
    port (
      c : in  std_logic;
      d : in  std_logic;
      e : in  std_logic;
      f : in  std_logic;
      i1 : in  coaxsig;
      i2 : in  std_logic;
      tp : out std_logic;
      q1 : out std_logic;
      q2 : out std_logic;
      q3 : out std_logic);

  end component;

  signal a : std_logic;
  signal b : std_logic;
  signal c : std_logic;
  signal d : std_logic;
  signal e : std_logic;
  signal f : std_logic;

begin -- gates
  u1 : kaslice1 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p6,
    i2 => p8,
    q1 => p1,
    q2 => p3,
    q3 => p4,
    tp => tp1);


  u2 : kaslice1 port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p24,
    i2 => p26,
    q1 => p23,
    q2 => p25,
    q3 => p28,
    tp => tp6);


  u3 : kaslice2 port map (
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p7,
    i2 => p9,
    q1 => p11,
    q2 => p10,
    q3 => p12,
    tp => tp2);


  u4 : kaslice2 port map (
    c => c,
    d => d,
    e => e,
    f => f,
    i1 => p21,
    i2 => p19,
    q1 => p17,
    q2 => p22,
    q3 => p20,
    tp => tp5);


  u5 : inv2 port map (
    a => p16,
    y => a,
    y2 => b);


  u6 : inv port map (
    a => p15,
    y => c);


  u7 : inv port map (
    a => p18,
    y => d);


  u8 : inv port map (
    a => p14,
    y => e);


  u9 : inv port map (
    a => p13,
    y => f);


  p27 <= v1;

end gates;

