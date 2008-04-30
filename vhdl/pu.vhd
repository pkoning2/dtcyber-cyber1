-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PU module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity puslice is
  
  port (
    s, r, e : in  std_logic;
    tp      : out std_logic;            -- test point
    q1, q2  : out std_logic;            -- q outputs
    qb      : out std_logic);           -- qb output

end puslice;

architecture gates of puslice is
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
  signal ri, si, qi, qbi : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => s,
    b => e,
    y => si);
  u2 : g2 port map (
    a => r,
    b => e,
    y => ri);
  u3 : rsflop port map (
    r  => ri,
    s  => si,
    q  => qi,
    qb => qbi);
  tp <= qi;
  u4 : inv2 port map (
    a  => qi,
    y  => q1,
    y2 => q2);
  u5 : inv2 port map (
    a  => qbi,
    y2 => qb);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pu is
  
  port (
    p10, p11, p17, p19, p18           : in  std_logic;
    tp1, tp5, tp6                     : out std_logic;  -- test points
    p9, p8, p1, p4, p3, p6, p7        : out std_logic;
    p14, p16, p15                     : inout misc;  -- analog stuff
    p22, p21, p28, p27, p26, p25, p24 : out std_logic);

end pu;

architecture gates of pu is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component puslice
    port (
      s, r, e : in  std_logic;
      tp      : out std_logic;            -- test point
      q1, q2  : out std_logic;            -- q outputs
      qb      : out std_logic);           -- qb output
  end component;
  signal a, t1, t2 : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p17,
    y2 => a);
  tp5 <= a;
  u2 : puslice port map (
    s  => p10,
    r  => p11,
    e  => a,
    tp => tp1,
    q1 => p9,
    q2 => p8,
    qb => t1);
  p1 <= t1;
  p4 <= t1;
  p3 <= t1;
  p6 <= t1;
  p7 <= t1;
  u3 : puslice port map (
    s  => p19,
    r  => p18,
    e  => a,
    tp => tp6,
    q1 => p22,
    q2 => p21,
    qb => t2);
  p28 <= t2;
  p27 <= t2;
  p26 <= t2;
  p25 <= t2;
  p24 <= t2;
  
end gates;
