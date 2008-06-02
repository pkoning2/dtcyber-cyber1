-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- AA module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity aaslice is
  
  port (
    s1, s2, r1, r2 : in  std_logic;
    tp             : out std_logic;     -- test point
    q1, q2, qb     : out std_logic);

end aaslice;

architecture gates of aaslice is
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
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => s1,
    b => s2,
    y => t1);
  u2 : g2 port map (
    a => r1,
    b => r2,
    y => t2);
  u3 : rsflop port map (
    s  => t1,
    r  => t2,
    q  => t3,
    qb => qb);
  tp <= t3;
  u4 : inv2 port map (
    a  => t3,
    y2 => t4);
  q1 <= t4;
  q2 <= t4;
  
end gates;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity aa is
  
  port (
    p5, p7, p6, p8, p12, p14, p11, p13     : in  std_logic;
    p15, p17, p16, p18, p22, p24, p21, p23 : in  std_logic;
    tp1, tp2, tp5, tp6                     : out std_logic;  -- test points
    p2, p4, p9, p1, p3, p10                : out std_logic;
    p26, p28, p19, p25, p27, p20           : out std_logic);

end aa;

architecture gates of aa is
  component aaslice
    port (
      s1, s2, r1, r2 : in  std_logic;
      tp             : out std_logic;     -- test point
      q1, q2, qb     : out std_logic);
  end component;
begin  -- gates

  u1 : aaslice port map (
    s1 => p5,
    s2 => p7,
    r1 => p6,
    r2 => p8,
    tp => tp1,
    q1 => p2,
    q2 => p4,
    qb => p9);
  u2 : aaslice port map (
    s1 => p12,
    s2 => p14,
    r1 => p11,
    r2 => p13,
    tp => tp2,
    q1 => p1,
    q2 => p3,
    qb => p10);
  u3 : aaslice port map (
    s1 => p15,
    s2 => p17,
    r1 => p16,
    r2 => p18,
    tp => tp5,
    q1 => p26,
    q2 => p28,
    qb => p19);
-- Note: p24 (second set input) was shown as p14 in the schematic,
-- obviously an error because p14 appears earlier.  I assume p24 was
-- meant, that would fit the usual pin layout patterns.
  u4 : aaslice port map (
    s1 => p22,
    s2 => p24,
    r1 => p21,
    r2 => p23,
    tp => tp6,
    q1 => p25,
    q2 => p27,
    qb => p20);

end gates;
