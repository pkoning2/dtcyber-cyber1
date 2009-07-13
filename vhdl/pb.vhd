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
-- PB module rev D -- barrel latches
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pbslice is
  
  port (
    i          : in  std_logic;         -- input
    c1, c2, c3 : in  std_logic;         -- clocks for the 3 stages
    tp1, tp2   : out std_logic;         -- test points
    q          : out std_logic);        -- output

end pbslice;

architecture gates of pbslice is
  component latch
    port (
      d, clk : in  std_logic;                 -- data (set), clock
      r      : in  std_logic := '0';          -- optional reset
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  signal q1, q2 : std_logic;     -- stage outputs
begin  -- gates
  u1 : latch port map (
    d   => i,
    clk => c1,
    q   => q1);
  tp1 <= q1;
  u2 : latch port map (
    d   => q1,
    clk => c2,
    q   => q2);
  tp2 <= q2;
  u3 : latch port map (
    d   => q2,
    clk => c3,
    q   => q);
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pb is
  
  port (
    p7, p6, p10                  : in  std_logic;    -- inputs
    p5, p22, p27                 : in  std_logic;    -- stage clocks
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;    -- test points
    p9, p3, p1                   : out std_logic);   -- outputs

end pb;

architecture gates of pb is
  component pbslice
    port (
      i          : in  std_logic;         -- input
      c1, c2, c3 : in  std_logic;         -- clocks for the 3 stages
      tp1, tp2   : out std_logic;         -- test points
      q          : out std_logic);        -- output
  end component;
begin  -- gates
  u1 : pbslice port map (
    i   => p7,
    c1  => p5,
    c2  => p22,
    c3  => p27,
    tp1 => tp1,
    tp2 => tp4,
    q   => p9);
  u2 : pbslice port map (
    i   => p6,
    c1  => p5,
    c2  => p22,
    c3  => p27,
    tp1 => tp2,
    tp2 => tp5,
    q   => p3);
  u3 : pbslice port map (
    i   => p10,
    c1  => p5,
    c2  => p22,
    c3  => p27,
    tp1 => tp3,
    tp2 => tp6,
    q   => p1);
  

end gates;
