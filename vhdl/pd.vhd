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
-- PD module rev D
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pdslice is
  
  port (
    i : in  logicsig;                        -- input
    clk : in logicsig;                       -- clock
    tp : out logicsig;                       -- test point output
    q, qb : out logicsig);                   -- outputs

end pdslice;

architecture gates of pdslice is
  component inv2
    port (
      a : in  logicsig;                      -- input
      y, y2 : out logicsig);                     -- output
  end component;
  component latch
    port (
      d, clk : in  logicsig;                   -- data (set), clock
      q, qb  : out logicsig);                  -- q and q.bar
  end component;
  signal ti : logicsig;                      -- copy of test point
begin  -- gates

  u1 : latch port map (
    d => i,
    clk => clk,
    q => ti);
  tp <= ti;
  u2 : inv2 port map (
    a => ti,
    y => qb,
    y2 => q);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pd is
  
  port (
    p16 : in  logicsig;                       -- clock input
    p14, p13, p18, p17 : in  logicsig;  -- inputs
    tp1, tp2, tp5, tp6 : out logicsig;       -- test points
    p11, p7, p9, p6, p4 : out logicsig;  -- bit 1 outputs
    p12, p10, p8, p5, p3 : out logicsig;  -- bit 2 outputs
    p19, p20, p25, p23, p28 : out logicsig;  -- bit 3 outputs
    p22, p24, p26, p21, p27 : out logicsig);  -- bit 4 outputs
end pd;

architecture gates of pd is
  component pdslice
    port (
      i : in  logicsig;                      -- input
      clk : in logicsig;                     -- clock
      tp    : out logicsig;                  -- test point output
      q, qb : out logicsig);                 -- outputs
  end component;
  signal q1, q2, q3, q4 : logicsig;
  signal qb1, qb2, qb3, qb4 : logicsig;
begin  -- gates
  
  u1 : pdslice port map (
    i  => p14,
    clk => p16,
    tp => tp1,
    q  => q1,
    qb => qb1);
  p11 <= q1;
  p7 <= q1;
  p9 <= q1;
  p6 <= qb1;
  p4 <= qb1;
  u2 : pdslice port map (
    i   => p13,
    clk => p16,
    tp  => tp2,
    q   => q2,
    qb  => qb2);
  p12 <= q2;
  p10 <= q2;
  p8 <= q2;
  p5 <= qb2;
  p3 <= qb2;
  u3 : pdslice port map (
    i   => p18,
    clk => p16,
    tp  => tp5,
    q   => q3,
    qb  => qb3);
  p19 <= q3;
  p20 <= q3;
  p25 <= q3;
  p23 <= qb3;
  p28 <= qb3;
  u4 : pdslice port map (
    i   => p17,
    clk => p16,
    tp  => tp6,
    q   => q4,
    qb  => qb4);
  p22 <= q4;
  p24 <= q4;
  p26 <= q4;
  p21 <= qb4;
  p27 <= qb4;
         
end gates;

