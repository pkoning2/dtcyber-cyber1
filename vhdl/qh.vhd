-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QH module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qhslice is
  
  port (
    a, c : in  std_logic;
    b, d : in  std_logic := '1';
    e    : in  std_logic := '1';
    tp   : out std_logic;               -- test point
    y    : out std_logic);              -- output

end qhslice;

architecture gates of qhslice is
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
  signal t1, t2, t3 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);
  u2 : g2 port map (
    a => c,
    b => d,
    y => t2);
  u3 : g3 port map (
    a => t1,
    b => t2,
    c => e,
    y => t3);
  tp <= t3;
  y <= t3;
  
end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qh is
  
  port (
    p10, p12, p5, p7, p9         : in  std_logic;
    p2, p4, p6, p8               : in  std_logic;
    p13, p15, p17, p14           : in  std_logic;
    p21, p23, p25, p27, p26      : in  std_logic;
    p18, p20, p22, p24           : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;   -- test points
    p3, p11, p1, p16, p28, p19   : out std_logic);  -- outputs

end qh;

architecture gates of qh is
  component qhslice
    port (
      a, c : in  std_logic;
      b, d : in  std_logic := '1';
      e    : in  std_logic := '1';
      tp   : out std_logic;               -- test point
      y    : out std_logic);              -- output
  end component;
begin  -- gates

  u1 : qhslice port map (
    a  => p10,
    c  => p12,
    e  => p5,
    tp => tp1,
    y  => p3);
  u2 : qhslice port map (
    a  => p7,
    c  => p9,
    tp => tp2,
    y  => p11);
  u3 : qhslice port map (
    a  => p2,
    b  => p4,
    c  => p6,
    d  => p8,
    tp => tp3,
    y  => p1);
  u4 : qhslice port map (
    a  => p13,
    b  => p15,
    c  => p17,
    e  => p14,
    tp => tp4,
    y  => p16);
  u5 : qhslice port map (
    a  => p21,
    b  => p23,
    c  => p25,
    d  => p27,
    e  => p26,
    tp => tp5,
    y  => p28);
  u6 : qhslice port map (
    a  => p18,
    b  => p20,
    c  => p22,
    d  => p24,
    tp => tp6,
    y  => p19);

end gates;
