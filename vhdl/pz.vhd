-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PZ module, rev E -- memory inhibit driver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pzslice is
  
  port (
    d, clk, r : in  std_logic;
    tp        : out std_logic;            -- test point
    q         : out std_logic);

end pzslice;

architecture gates of pzslice is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      s2, s3, s4, r2, r3, r4  : in  std_logic := '1';-- extra set, reset if needed
      q, qb : out std_logic);                 -- q and q.bar
  end component;
  signal t1, t2 : std_logic;
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
    p4, p2, p6, p19, p27, p22    : in  std_logic;
    p17, p9, p16, p10            : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p5, p3, p7, p21, p25, p23    : out std_logic);

end pz;

architecture gates of pz is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component pzslice
    port (
      d, clk, r : in  std_logic;
      tp        : out std_logic;            -- test point
      q         : out std_logic);
  end component;
  signal a, c, t1 : std_logic;
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
