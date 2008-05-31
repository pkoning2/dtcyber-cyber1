-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PT module, rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pt is
  
  port (
    p1, p3, p5, p27, p25, p23, p7 : in  std_logic;
    p4, p21                       : in  std_logic;
    tp1, tp2, tp5, tp6            : out std_logic;  -- test points
    p9, p11, p17, p19, p15        : out std_logic);

end pt;

architecture gates of pt is
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
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal a, b, c, d, e, g, k : std_logic;
  signal h, i, j, n, m, l, p : std_logic;
  signal t1, t2, t3, t4, t5, t6, t7, t8, t9, t10 : std_logic;
begin  -- gates

  u1 : g4 port map (
    a => a,
    b => j,
    c => c,
    d => b,
    y => t1);
  u2 : g2 port map (
    a => d,
    b => a,
    y => t2);
  u3 : g2 port map (
    a => t1,
    b => t2,
    y => t3);
  tp1 <= t3;
  u4 : g2 port map (
    a => k,
    b => t3,
    y => t4);
  p9 <= t4;
  p11 <= t4;
  u5 : g2 port map (
    a => t3,
    b => p4,
    y => t5);
  tp2 <= t5;
  u6 : g2 port map (
    a => t5,
    b => k,
    y => p17);
  u7 : inv2 port map (
    a  => p1,
    y  => h,
    y2 => b);
  u8 : inv2 port map (
    a  => p3,
    y  => i,
    y2 => c);
  u9 : inv2 port map (
    a  => p5,
    y  => j,
    y2 => d);
  u10 : inv2 port map (
    a  => p27,
    y  => n,
    y2 => e);
  u11 : inv2 port map (
    a  => p25,
    y  => m,
    y2 => g);
  u12 : inv port map (
    a => p23,
    y => l);
  u13 : inv2 port map (
    a  => p7,
    y  => p,
    y2 => k);
  u14 : g3 port map (
    a  => l,
    b  => m,
    c  => n,
    y  => tp5,
    y2 => a);
  u15 : g4 port map (
    a => a,
    b => j,
    c => c,
    d => h,
    y => t6);
  u16 : g4 port map (
    a => a,
    b => j,
    c => i,
    d => b,
    y => t7);
  u17 : g3 port map (
    a => g,
    b => l,
    c => e,
    y => t8);
  u18 : g3 port map (
    a  => t6,
    b  => t7,
    c  => t8,
    y  => t9,
    y2 => p15);
  tp6 <= t9;
  u19 : inv port map (
    a => p21,
    y => t10);
  u20 : g3 port map (
    a => t10,
    b => t9,
    c => k,
    y => p19);
  
end gates;
