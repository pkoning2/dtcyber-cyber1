-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity tgslice is
  
  port (
    i1, i2, i3, i4 : in  std_logic;           -- inputs
    a, b, c, d     : in  std_logic;           -- selectors
    q, qb          : out std_logic;           -- outputs
    tp             : out std_logic);          -- test point

end tgslice;

architecture bool of tgslice is
  signal ti : std_logic;                      -- internal copy of output
  signal ii1, ii2, ii3, ii4 : std_logic;
begin  -- bool

  -- We could do this as gates, but chances are that writing
  -- it this way will result in better synthesis
  ii1 <= '1' when i1 = 'U' else i1;
  ii2 <= '1' when i2 = 'U' else i2;
  ii3 <= '1' when i3 = 'U' else i3;
  ii4 <= '1' when i4 = 'U' else i4;
  ti <= ii1 when a = '1' else
        ii2 when b = '1' else
        ii3 when c = '1' else
        ii4 when d = '1' else '0';
  tp <= ti after 2 * t;
  q  <= ti after 2 * t;
  qb <= not (ti) after 3 * t;
  
end bool;

library IEEE;
use IEEE.std_logic_1164.all;

entity tg is
  
  port (
    p2, p4, p6, p8     : in  std_logic := '1';   -- bit 0 inputs
    p13, p14           : out std_logic;   -- bit 0 outputs (true/complement)
    tp1                : out std_logic;   -- test point 1
    p12, p16, p11, p15 : in  std_logic := '1');  -- selects a, b, c, d

end tg;

architecture gates of tg is
  component inv
    port (
      i : in  std_logic;                      -- input
      o : out std_logic);                     -- output
  end component;
  component tgslice
    port (
      i1, i2, i3, i4 : in  std_logic;           -- inputs
      a, b, c, d     : in  std_logic;           -- selectors
      q, qb          : out std_logic;           -- outputs
      tp             : out std_logic);          -- test point
  end component;
  signal a, b, c, d : std_logic;        -- internal selects
begin  -- gates

  u1 : inv port map (
    i => p12,
    o => a);
  u2 : inv port map (
    i => p16,
    o => b);
  u3 : inv port map (
    i => p11,
    o => c);
  u4 : inv port map (
    i => p15,
    o => d);
  u5 : tgslice port map (
    i1 => p2,
    i2 => p4,
    i3 => p6,
    i4 => p8,
    a  => a,
    b  => b,
    c  => c,
    d  => d,
    q  => p13,
    qb => p14,
    tp => tp1);

end gates;
