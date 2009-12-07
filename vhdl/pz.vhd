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
-- PZ module, rev E -- memory inhibit driver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pzslice is
  
  port (
    d, clk, r : in  logicsig;
    tp        : out logicsig;            -- test point
    q         : out logicsig);

end pzslice;

architecture gates of pzslice is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal t1, t2 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => d,
    b => clk,
    y => t1);
  u2 : rsflop port map (
    s => t1,
    r => r,
    q => t2);
  tp <= t2;
  u3 : inv2 port map (
    a  => t2,
    y2 => q);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pz is
  
  port (
    p4, p2, p6, p19, p27, p22    : in  logicsig;
    p17, p9, p16, p10            : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6 : out logicsig;  -- test points
    p5, p3, p7, p21, p25, p23    : out logicsig);

end pz;

architecture gates of pz is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component pzslice
    port (
      d, clk, r : in  logicsig;
      tp        : out logicsig;            -- test point
      q         : out logicsig);
  end component;
  signal a, c, t1 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a  => p17,
    b  => p9,
    y2 => a);
  u2 : g2 port map (
    a  => p16,
    b  => p10,
    y2 => t1);
  u9 : inv port map (
    a => t1,
    y => c);
  u3 : pzslice port map (
    d   => p4,
    clk => a,
    r   => c,
    tp  => tp1,
    q   => p5);
  u4 : pzslice port map (
    d   => p2,
    clk => a,
    r   => c,
    tp  => tp2,
    q   => p3);
  u5 : pzslice port map (
    d   => p6,
    clk => a,
    r   => c,
    tp  => tp3,
    q   => p7);
  u6 : pzslice port map (
    d   => p19,
    clk => a,
    r   => c,
    tp  => tp4,
    q   => p21);
  u7 : pzslice port map (
    d   => p27,
    clk => a,
    r   => c,
    tp  => tp5,
    q   => p25);
  u8 : pzslice port map (
    d   => p22,
    clk => a,
    r   => c,
    tp  => tp6,
    q   => p23);
end gates;
