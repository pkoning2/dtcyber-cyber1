-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- PQ module, rev D -- deadstart panel control and central busy state
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pq is
  
  port (
    p6, p22, p24, p26, p28, p11, p17, p19  : in  std_logic;
    p1, p5, p21                            : in  coaxsig;
    p23, p25, p3                           : in std_logic;
    -- initial value of 0 is temporary (for switches not connected)
    p4, p2, p16                            : in  std_logic := '0';
    tp1, tp2, tp5, tp6                     : out std_logic;  -- test points
    p27, p13, p18, p20, p10, p8            : out std_logic;
    p14, p9, p12, p7, p15                  : out std_logic);

end pq;

architecture gates of pq is
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
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                        -- source
      y : out std_logic);                     -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      s2, s3, s4, r2, r3, r4  : in  std_logic := '1';-- extra set, reset if needed
      q, qb : out std_logic);                 -- q and q.bar
  end component;
  signal a, g, h, j, k : std_logic;
  signal t1, t2, t3, t4, t5, t6, t7, t8 : std_logic;
  signal t9, t10, t11, t12, t13, t14, t15, t16 : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p6,
    y2 => t1);
  u2 : g2 port map (
    a => t13,
    b => t1,
    y => t2);
  u3 : g2 port map (
    a => p23,
    b => t1,
    y => t3);
  u4 : g2 port map (
    a => p25,
    b => t1,
    y => t4);
  u5 : cxreceiver port map (
    a => p1,
    y => t5);
  u6 : cxreceiver port map (
    a => p5,
    y => t6);
  u7 : cxreceiver port map (
    a => p21,
    y => t7);
  u8 : rsflop port map (
    s  => t2,
    s2 => t3,
    s3 => t4,
    r  => t5,
    r2 => t6,
    r3 => t7,
    r4 => p3,
    q  => t8);
  tp1 <= t8;
  u9 : inv2 port map (
    a  => t8,
    y2 => t9);
  p27 <= t9;
  p13 <= t9;
  u10 : inv port map (
    a => t9,
    y => t10);
  u11 : g4 port map (
    a  => p22,
    b  => p24,
    c  => p26,
    d  => p28,
    y2 => a);
  tp5 <= a;
  u12 : inv2 port map (
    a  => p11,
    y  => h,
    y2 => g);
  u13 : inv port map (
    a => p17,
    y => j);
  u14 : inv port map (
    a => p19,
    y => k);
  u15 : g4 port map (
    a  => g,
    b  => a,
    c  => j,
    d  => k,
    y2 => t11);
  tp6 <= t11;
  u16 : inv port map (
    a => t11,
    y => p10);
  u17 : g2 port map (
    a  => t10,
    b  => t11,
    y2 => t12);
  p18 <= t12;
  u18 : inv2 port map (
    a  => t12,
    y  => p20,
    y2 => t13);
  u19 : g4 port map (
    a  => a,
    b  => h,
    c  => j,
    d  => k,
    y  => p14,
    y2 => t14);
  tp2 <= t14;
  u20 : inv port map (
    a => t14,
    y => p8);
  u21 : inv port map (
    a => p4,
    y => t15);
  p9 <= t15;
  u22 : inv port map (
    a => p2,
    y => t16);
  p7 <= t16;
  u23 : g2 port map (
    a => t15,
    b => t16,
    y => p12);
  p15 <= p16;

end gates;
