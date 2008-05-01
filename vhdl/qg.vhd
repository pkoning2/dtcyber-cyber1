-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QG module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qg is
  
  port (
    p1, p3, p5, p25, p23         : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p13, p12, p10, p11, p9, p7   : out std_logic;
    p16, p18, p20, p15, p17, p27 : out std_logic;
    p22, p26, p28, p8, p4, p2    : out std_logic);

end qg;

architecture gates of qg is
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
  component latch
    port (
      d, clk : in  std_logic;                 -- data (set), clock
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  signal c, d, e : std_logic;
  signal t1, t2, t3, t4, t5 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => p1,
    b => p3,
    y => t1);
  u2 : latch port map (
    d   => t1,
    clk => p5,
    q   => c);
  tp1 <= c;
  u3 : inv2 port map (
    a => c,
    y => t2);
  p13 <= t2;
  p12 <= t2;
  p10 <= t2;
  p11 <= t2;
  p9 <= t2;
  p7 <= t2;
  u4 : g2 port map (
    a => p25,
    b => p23,
    y => t3);
  u5 : latch port map (
    d   => t3,
    clk => p5,
    q   => d);
  tp6 <= d;
  u6 : inv2 port map (
    a  => d,
    y2 => t4);
  p16 <= t4;
  p18 <= t4;
  p20 <= t4;
  p15 <= t4;
  p17 <= t4;
  p27 <= t4;
  u7 : g2 port map (
    a => c,
    b => d,
    y => e);
  tp6 <= e;
  u8 : inv2 port map (
    a => e,
    y => t5);
  p22 <= t5;
  p26 <= t5;
  p28 <= t5;
  p8 <= t5;
  p4 <= t5;
  p2 <= t5;
  
end gates;
