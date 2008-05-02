-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PE module rev D
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pe is
  
  port (
    p4, p6, p25, p26, p10, p21, p3, p8 : in  std_logic;
    p23, p22, p5, p24                  : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6       : out std_logic;  -- test points
    p9, p15, p11                       : out std_logic;
    p17, p28, p27, p20                 : out std_logic;
    p18, p19, p12, p1, p13, p14        : out std_logic);

end pe;

architecture gates of pe is
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
  component g5
    port (
      a, b, c, d, e : in  std_logic;          -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component g6
    port (
      a, b, c, d, e, f : in  std_logic;       -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal i, a, b, d, e, f : std_logic;
  signal x, p, u, s, t, v, w, r : std_logic;
  signal t1 : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p4,
    y  => i,
    y2 => a);
  u2 : inv2 port map (
    a  => p6,
    y  => e,
    y2 => f);
  u3 : inv2 port map (
    a  => p25,
    y2 => b);
  u4 : inv port map (
    a => p26,
    y => d);
  u5 : g3 port map (
    a  => b,
    b  => p10,
    c  => a,
    y  => x,
    y2 => p9);
  u6 : g3 port map (
    a  => b,
    b  => p21,
    c  => f,
    y  => p,
    y2 => p15);
  u7 : g3 port map (
    a  => b,
    b  => p3,
    c  => i,
    y  => u,
    y2 => p11);
  u8 : g4 port map (
    a => b,
    b => p8,
    c => e,
    d => a,
    y => s);
  u9 : g6 port map (
    a  => p,
    b  => s,
    c  => u,
    d  => v,
    e  => w,
    f  => x,
    y  => tp1,
    y2 => p17);
  u10 : g5 port map (
    a => b,
    b => p23,
    c => d,
    d => e,
    e => a,
    y => t);
  tp2 <= t;
  u11 : g3 port map (
    a => b,
    b => p22,
    c => i,
    y => v);
  tp3 <= v;
  u12 : g3 port map (
    a => b,
    b => p5,
    c => i,
    y => w);
  tp4 <= w;
  u13 : g4 port map (
    a => b,
    b => p24,
    c => d,
    d => f,
    y => r);
  tp5 <= r;
  u14 : inv2 port map (
    a  => r,
    y2 => t1);
  p1 <= t1;
  p13 <= t1;
  p14 <= t1;
  u15 : g3 port map (
    a  => r,
    b  => t,
    c  => w,
    y  => tp6,
    y2 => p28);
  u16 : g2 port map (
    a  => r,
    b  => t,
    y  => p27,
    y2 => p20);
  u17 : g3 port map (
    a => b,
    b => d,
    c => i,
    y => p18);
  u18 : g2 port map (
    a => p,
    b => r,
    y => p19);
  u19 : g2 port map (
    a => v,
    b => w,
    y => p12);
  
end gates;
