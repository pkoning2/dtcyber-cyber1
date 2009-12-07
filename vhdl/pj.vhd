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
-- PJ module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pjslice is
  
  port (
    d, clk     : in  logicsig;             -- data and clock
    tp         : out logicsig;             -- test point
    qb, q1, q2 : out logicsig);            -- outputs

end pjslice;

architecture gates of pjslice is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2  : out logicsig);                    -- output
  end component;
  component latch
    port (
      d, clk : in  logicsig;                 -- data (set), clock
      q, qb  : out logicsig);                -- q and q.bar
  end component;
  signal tqi, tqi2 : logicsig;                 -- latch output
begin  -- gates
  u1 : latch port map (
    d   => d,
    clk => clk,
    q   => tqi);
  tp <= tqi;
  u2 : inv2 port map (
    a => tqi,
    y => qb,
    y2 => tqi2);
  q1 <= tqi2;
  q2 <= tqi2;
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pj is
  
  port (
    p8, p5, p12, p21, p24, p23                  : in  logicsig;  -- inputs
    p9, p7, p20, p22                            : in  logicsig;  -- clocks
    tp1, tp2, tp3, tp4, tp5, tp6                : out logicsig;  -- test points
    p4, p1, p3, p11, p2, p6, p13, p14, p10      : out logicsig;
    p15, p19, p17, p16, p27, p18, p25, p28, p26 : out logicsig);

end pj;

architecture gates of pj is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2    : out logicsig);                  -- output
  end component;
  component pjslice
    port (
      d, clk     : in  logicsig;             -- data and clock
      tp         : out logicsig;             -- test point
      qb, q1, q2 : out logicsig);            -- outputs
  end component;
  signal b, d : logicsig;       -- strobes
begin  -- gates
  u1 : g2 port map (
    a => p9,
    b => p7,
    y2 => b);
  u2 : g2 port map (
    a => p20,
    b => p22,
    y2 => d);
  u3 : pjslice port map (
    d   => p8,
    clk => b,
    tp  => tp1,
    qb  => p4,
    q1  => p1,
    q2  => p3);
  u4 : pjslice port map (
    d   => p5,
    clk => b,
    tp  => tp2,
    qb  => p11,
    q1  => p2,
    q2  => p6);
  u5 : pjslice port map (
    d   => p12,
    clk => b,
    tp  => tp3,
    qb  => p13,
    q1  => p14,
    q2  => p10);
  u6 : pjslice port map (
    d   => p21,
    clk => d,
    tp  => tp4,
    qb  => p15,
    q1  => p19,
    q2  => p17);
  u7 : pjslice port map (
    d   => p24,
    clk => d,
    tp  => tp5,
    qb  => p16,
    q1  => p27,
    q2  => p18);
  u8 : pjslice port map (
    d   => p23,
    clk => d,
    tp  => tp6,
    qb  => p25,
    q1  => p28,
    q2  => p26);

end gates;
