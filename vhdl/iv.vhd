-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- IV module, 3 to 8 decode with true and complement outputs
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity iv is
  
  port (
    p6, p8, p26                           : in  std_logic;
    tp1, tp2, tp5, tp6                    : out std_logic;  -- test points
    p23, p1, p21, p20, p17, p4, p2, p28   : out std_logic;
    p16, p10, p14, p11, p12, p9, p13, p15 : out std_logic);

end iv;

architecture gates of iv is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal a, b, c, d, e, f : std_logic;
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p6,
    y  => a,
    y2 => b);
  u2 : inv2 port map (
    a  => p8,
    y  => c,
    y2 => d);
  u3 : inv2 port map (
    a  => p26,
    y  => e,
    y2 => f);
  u4 : g3 port map (
    a  => a,
    b  => c,
    c  => e,
    y  => t1,
    y2 => p16);
  tp6 <= t1;
  p23 <= t1;
  u5 : g3 port map (
    a  => b,
    b  => c,
    c  => e,
    y  => p1,
    y2 => p10);
  u6 : g3 port map (
    a  => a,
    b  => d,
    c  => e,
    y  => t2,
    y2 => p14);
  p21 <= t2;
  tp1 <= t2;
  u7 : g3 port map (
    a  => b,
    b  => d,
    c  => e,
    y  => t3,
    y2 => p11);
  p20 <= t3;
  tp5 <= t3;
  u8 : g3 port map (
    a  => a,
    b  => c,
    c  => f,
    y  => t4,
    y2 => p12);
  p17 <= t4;
  tp2 <= t4;
  u9 : g3 port map (
    a  => b,
    b  => c,
    c  => f,
    y  => p4,
    y2 => p9);
  u10 : g3 port map (
    a  => a,
    b  => d,
    c  => f,
    y  => p2,
    y2 => p13);
  u11 : g3 port map (
    a  => b,
    b  => d,
    c  => f,
    y  => p28,
    y2 => p15);

end gates;
