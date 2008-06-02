-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- XI module -- 6612 keyboard input buffers
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity xi is
  
  port (
    p1, p24, p4, p25, p5, p28    : in  std_logic;
    p20, p9                      : in  std_logic;   -- latch clocks
    p21, p11                     : in  std_logic;   -- clear
    p19, p12                     : in  std_logic;   -- key down, key up
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;   -- test points
    p3, p22, p6, p23, p7, p26    : out std_logic;
    p17, p14                     : out std_logic);  -- key down, key up

end xi;

architecture gates of xi is
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
      r      : in  std_logic := '0';          -- optional reset
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  signal c : std_logic;
  signal t1, t2, t3, t4, t5, t6 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a  => p21,
    b  => p11,
    y2 => c);
  u2 : inv2 port map (
    a  => p19,
    y2 => p17);
  u3 : inv2 port map (
    a  => p12,
    y2 => p14);
  u4 : latch port map (
    d   => p1,
    clk => p9,
    r   => c,
    q   => t1);
  tp1 <= t1;
  p3 <= t1;
  u5 : latch port map (
    d   => p24,
    clk => p20,
    r   => c,
    q   => t2);
  tp4 <= t2;
  p22 <= t2;
  u6 : latch port map (
    d   => p4,
    clk => p9,
    r   => c,
    q   => t3);
  tp2 <= t3;
  p6 <= t3;
  u7 : latch port map (
    d   => p25,
    clk => p20,
    r   => c,
    q   => t4);
  tp5 <= t4;
  p23 <= t4;
  u8 : latch port map (
    d   => p5,
    clk => p9,
    r   => c,
    q   => t5);
  tp3 <= t5;
  p7 <= t5;
  u9 : latch port map (
    d   => p28,
    clk => p20,
    r   => c,
    q   => t6);
  tp6 <= t6;
  p26 <= t6;
  
end gates;
