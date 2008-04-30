-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QK module -- incrementer
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qkslice is
  
  port (
    k      : in  std_logic;             -- control input
    k2, k3 : in std_logic := '1';       -- additional control inputs
    a, b   : in  std_logic;             -- inputs
    tp     : out std_logic;             -- test point
    y      : out std_logic);

end qkslice;

architecture gates of qkslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3, t4, t5 : std_logic;
begin  -- gates

  u1 : g3 port map (
    a  => k,
    b  => k2,
    c  => k3,
    y  => t1,
    y2 => t2);
  u2 : g2 port map (
    a => b,
    b => t2,
    y => t3);
  u3 : g2 port map (
    a => t1,
    b => a,
    y => t4);
  u4 : g2 port map (
    a => t3,
    b => t4,
    y => t5);
  tp <= t5;
  y <= t5;

end gates;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qk is
  
  port (
    p13, p14, p18                : in  std_logic;
    p26, p25, p28, p27           : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p9, p24, p20, p3, p5, p7     : out std_logic);

end qk;

architecture gates of qk is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
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
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component qkslice
    port (
      k      : in  std_logic;             -- control input
      k2, k3 : in std_logic := '1';       -- additional control inputs
      a, b   : in  std_logic;             -- inputs
      tp     : out std_logic;             -- test point
      y      : out std_logic);
  end component;
  signal a, b, c, d, e, f, k, t : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p13,
    y2 => a);
  tp3 <= a;
  u2 : inv port map (
    a => a,
    y => b);
  u3 : inv2 port map (
    a  => p14,
    y2 => c);
  tp2 <= c;
  u4 : inv port map (
    a => c,
    y => d);
  u5 : inv2 port map (
    a  => p18,
    y2 => e);
  tp4 <= e;
  u6 : inv port map (
    a => e,
    y => f);
  u7 : g4 port map (
    a  => p26,
    b  => p25,
    c  => p28,
    d  => p27,
    y2 => k);
  u8 : qkslice port map (
    k  => k,
    b  => b,
    a  => a,
    tp => tp1,
    y  => p9);
  u9 : qkslice port map (
    k  => k,
    k2 => a,
    a  => c,
    b  => d,
    tp => tp6,
    y  => p24);
  u10 : qkslice port map (
    k  => k,
    k2 => a,
    k3 => c,
    a  => e,
    b  => f,
    tp => tp5,
    y  => p20);
  u11 : g3 port map (
    a  => a,
    b  => c,
    c  => e,
    y2 => t);
  p3 <= t;
  p5 <= t;
  p7 <= t;
  
end gates;
