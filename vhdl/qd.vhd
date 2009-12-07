-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- QD module, rev C -- add/boolean ops control
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qdslice is
  
  port (
    a, b, c, d, e, f, g : in  logicsig;   -- inputs
    ca, cb, cc, cd      : in  logicsig;   -- controls
    tp1, tp2            : out logicsig;   -- test points
    y                   : out logicsig);  -- output

end qdslice;

architecture qdslice of qdslice is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  logicsig;             -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2, t3, t4, t5, t6, t7, t8 : logicsig;
begin  -- qdslice

  u1 : g2 port map (
    a => cc,
    b => a,
    y => t1);
  u2 : g2 port map (
    a  => t1,
    b  => b,
    y  => t2,
    y2 => t3);
  tp2 <= t3;
  u3 : g3 port map (
    a => t2,
    b => c,
    c => ca,
    y => t4);
  u4 : g3 port map (
    a => t3,
    b => d,
    c => ca,
    y => t5);
  u5 : g2 port map (
    a => t3,
    b => cb,
    y => t6);
  u6 : g4 port map (
    a => cd,
    b => e,
    c => f,
    d => g,
    y => t7);
  u7 : g4 port map (
    a => t7,
    b => t6,
    c => t4,
    d => t5,
    y => t8);
  tp1 <= t8;
  y <= t8;
  
end qdslice;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qd is
  
  port (
    p11, p4, p9, p6, p20, p27, p22, p25 : in  logicsig;
    p8, p18, p23, p13                   : in  logicsig;
    p7, p3, p5, p24, p26, p28           : in  logicsig;
    p21                                 : in  logicsig;
    tp1, tp2, tp5, tp6                  : out logicsig;  -- test points
    p12, p19, p10, p15, p16             : out logicsig);

end qd;

architecture gates of qd is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component qdslice
    port (
      a, b, c, d, e, f, g : in  logicsig;   -- inputs
      ca, cb, cc, cd      : in  logicsig;   -- controls
      tp1, tp2            : out logicsig;   -- test points
      y                   : out logicsig);  -- output
  end component;
  signal a, b, c, d, t1 : logicsig;
begin  -- gates

  u1 : inv port map (
    a => p8,
    y => a);
  u2 : inv port map (
    a => p18,
    y => b);
  u3 : inv port map (
    a => p23,
    y => c);
  u4 : inv port map (
    a => p13,
    y => d);
  u5 : inv2 port map (
    a  => p21,
    y2 => t1);
  p10 <= t1;
  p15 <= t1;
  p16 <= t1;
  u6 : qdslice port map (
    a   => p11,
    b   => p4,
    c   => p9,
    d   => p6,
    e   => p7,
    f   => p3,
    g   => p5,
    ca  => a,
    cb  => b,
    cc  => c,
    cd  => d,
    tp1 => tp1,
    tp2 => tp2,
    y   => p12);
  u7 : qdslice port map (
    a   => p20,
    b   => p27,
    c   => p22,
    d   => p25,
    e   => p24,
    f   => p26,
    g   => p28,
    ca  => a,
    cb  => b,
    cc  => c,
    cd  => d,
    tp1 => tp6,
    tp2 => tp5,
    y   => p19);
  
end gates;
