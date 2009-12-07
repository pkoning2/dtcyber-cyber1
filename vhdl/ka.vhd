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
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

end kaslice1;
architecture gates of kaslice1 is
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
  signal t3 : logicsig;
  signal t4 : logicsig;
  signal t5 : logicsig;
  signal t6 : logicsig;

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
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

end kaslice2;
architecture gates of kaslice2 is
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
      p8 : in  logicsig;
      p9 : in  logicsig;
      p13 : in  logicsig;
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  coaxsig;
      p24 : in  coaxsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p10 : out logicsig;
      p11 : out logicsig;
      p12 : out logicsig;
      p17 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig;
      p28 : out logicsig);

end ka;
architecture gates of ka is
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

  component kaslice1
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

  end component;

  component kaslice2
    port (
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      i1 : in  coaxsig;
      i2 : in  logicsig;
      tp : out logicsig;
      q1 : out logicsig;
      q2 : out logicsig;
      q3 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;

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

