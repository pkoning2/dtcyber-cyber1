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
-- PC module rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pc is
  
  port (
    p7, p5, p8, p6, p23, p25, p24, p26 : in  logicsig;   -- inputs
    p22, p11                           : in  logicsig;   -- clocks
    p13, p20                           : in  logicsig;   -- selects
    tp1, tp2, tp5, tp6                 : out logicsig;   -- test points
    p14, p9, p18, p21                  : out logicsig;   -- outputs
    p2, p4, p15, p17                   : out logicsig);  -- pullups

end pc;

architecture gates of pc is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2    : out logicsig);                  -- output
  end component;
  component latchd4
    port (
      d, d2 : in  logicsig;             -- inputs
      e, e2 : in  logicsig;             -- enables
      clk   : in  logicsig;             -- clock
      q, qb : out logicsig);            -- output
  end component;
  signal e, f : logicsig;       -- selects
  signal q1, q2, q3, q4 : logicsig;    -- outputs
begin  -- gates
  p2 <= '0';
  p4 <= '0';
  p15 <= '0';
  p17 <= '0';
  u1 : inv port map (
    a => p13,
    y => e);
  u2 : g2 port map (
    a => e,
    b => p20,
    y => f);
  u3 : latchd4 port map (
    d  => p7,
    e  => e,
    d2  => p5,
    e2  => f,
    clk => p11,
    q   => q1);
  tp1 <= q1;
  p14 <= q1;
  u4 : latchd4 port map (
    d  => p8,
    e  => e,
    d2  => p6,
    e2  => f,
    clk => p22,
    q   => q2);
  tp2 <= q2;
  p9 <= q2;
  u5 : latchd4 port map (
    d  => p23,
    e  => e,
    d2  => p25,
    e2  => f,
    clk => p11,
    q   => q3);
  tp5 <= q3;
  p18 <= q3;
  u6 : latchd4 port map (
    d  => p24,
    e  => e,
    d2  => p26,
    e2  => f,
    clk => p22,
    q   => q4);
  tp6 <= q4;
  p21 <= q4;
end gates;
