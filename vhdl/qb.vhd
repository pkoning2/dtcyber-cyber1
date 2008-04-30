-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QB module
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qb is
  
  port (
    p7, p5, p3, p17, p19, p15    : in  std_logic;
    p11, p9, p13, p21, p25, p27  : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p2, p1, p20, p22, p24        : out std_logic;
    p4, p8, p6                   : out std_logic;
    p14, p16, p28, p12, p18, p26 : out std_logic);

end qb;

architecture gates of qb is
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
  signal a, b, c, d, e, f, x : std_logic;
  signal t1, t2, t3, t4, t5, t6, t7, t8 : std_logic;
  signal t9, t10, t11, t12, t13, t14, t15, t16 : std_logic;
  signal t17, t18, t19 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => p7,
    b => p5,
    y => a);
  u2 : inv2 port map (
    a  => p3,
    y  => f,
    y2 => t1);
  u3 : g2 port map (
    a => p17,
    b => p19,
    y => b);
  u4 : inv2 port map (
    a  => p15,
    y  => e,
    y2 => t2);
  u5 : g2 port map (
    a => p11,
    b => p9,
    y => a);
  u6 : inv2 port map (
    a  => p13,
    y  => d,
    y2 => t3);
  u7 : g2 port map (
    a => c,
    b => t1,
    y => t4);
  tp2 <= t4;
  p4 <= t4;
  u8 : g2 port map (
    a => b,
    b => t2,
    y => t5);
  tp3 <= t5;
  p8 <= t5;
  u9 : g2 port map (
    a => a,
    b => t3,
    y => t6);
  tp4 <= t6;
  p6 <= t6;
  u10 : g3 port map (
    a => p21,
    b => p25,
    c => p27,
    y => x);
  u11 : inv2 port map (
    a  => x,
    y  => p18,
    y2 => t7);
  tp6 <= t7;
  p26 <= t7;
  u12 : g2 port map (
    a => a,
    b => x,
    y => t8);
  u13 : inv port map (
    a => d,
    y => t9);
  u14 : g2 port map (
    a  => t8,
    b  => t9,
    y  => t10,
    y2 => p1);
  tp1 <= t10;
  p2 <= t10;
  u15 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t11);
  p20 <= t11;
  p22 <= t11;
  p24 <= t11;
  u16 : g2 port map (
    a => e,
    b => c,
    y => t12);
  u17 : inv port map (
    a => f,
    y => t13);
  u18 : g3 port map (
    a => c,
    b => b,
    c => d,
    y => t14);
  u19 : g3 port map (
    a => t12,
    b => t13,
    c => t14,
    y => t15);
  p14 <= t15;
  p16 <= t15;
  u20 : g3 port map (
    a => x,
    b => a,
    c => b,
    y => t16);
  u21 : g2 port map (
    a => d,
    b => b,
    y => t17);
  u22 : inv port map (
    a => e,
    y => t18);
  u23 : g3 port map (
    a  => t18,
    b  => t16,
    c  => t17,
    y  => t19,
    y2 => p12);
  tp5 <= t19;
  p28 <= t19;
  
end gates;
