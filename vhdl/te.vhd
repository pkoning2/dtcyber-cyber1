-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- TE module, rev C -- hex 3-input mux
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity teslice is
  
  port (
    a, b, c    : in  std_logic;         -- inputs
    e1, e2, e3 : in  std_logic;         -- enables
    tp         : out std_logic;         -- test point
    y          : out std_logic);        -- output

end teslice;

architecture gates of teslice is
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
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => e1,
    y => t1);
  u2 : g2 port map (
    a => b,
    b => e2,
    y => t2);
  u3 : g2 port map (
    a => c,
    b => e3,
    y => t3);
  u4 : g3 port map (
    a => t1,
    b => t2,
    c => t3,
    y => t4);
  tp <= t4;
  y <= t4;

end gates;
library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity te is
  
  port (
    p6, p4, p2, p7, p13, p12, p24, p26, p28  : in  std_logic;
    p5, p3, p1, p22, p18, p17, p23, p25, p27 : in  std_logic;
    p14, p15, p16                            : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6             : out std_logic;  -- test points
    p9, p10, p21, p8, p19, p20               : out std_logic);

end te;

architecture gates of te is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component teslice
    port (
      a, b, c    : in  std_logic;         -- inputs
      e1, e2, e3 : in  std_logic;         -- enables
      tp         : out std_logic;         -- test point
      y          : out std_logic);        -- output
  end component;
  signal a, b, c : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p14,
    y => a);
  u2 : inv port map (
    a => p15,
    y => b);
  u3 : inv port map (
    a => p16,
    y => c);
  u4 : teslice port map (
    a  => p6,
    b  => p4,
    c  => p2,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp1,
    y  => p9);
  u5 : teslice port map (
    a  => p5,
    b  => p3,
    c  => p1,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp2,
    y  => p8);
  u6 : teslice port map (
    a  => p7,
    b  => p13,
    c  => p12,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp3,
    y  => p10);
  u7 : teslice port map (
    a  => p22,
    b  => p18,
    c  => p17,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp4,
    y  => p19);
  u8 : teslice port map (
    a  => p24,
    b  => p26,
    c  => p28,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp5,
    y  => p21);
  u9 : teslice port map (
    a  => p23,
    b  => p25,
    c  => p27,
    e1 => a,
    e2 => b,
    e3 => c,
    tp => tp6,
    y  => p20);

end gates;

  
