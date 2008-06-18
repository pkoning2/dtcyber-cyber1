-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- QF module, rev D -- shift network
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qfslice is
  
  port (
    a, b, c    : in  std_logic;         -- inputs
    ca, cb, cc : in  std_logic;         -- controls
    tp         : out std_logic;         -- test point
    ya, yb, yc : out std_logic);        -- outputs

end qfslice;

architecture gates of qfslice is
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
  signal t1 : std_logic;
begin  -- gates

  u1 : g3 port map (
    a => a,
    b => b,
    c => c,
    y => t1);
  tp <= t1;
  u2 : g2 port map (
    a => ca,
    b => t1,
    y => ya);
  u3 : g2 port map (
    a => b,
    b => t1,
    y => yb);
  u4 : g2 port map (
    a => t1,
    b => c,
    y => yc);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qf is
  
  port (
    p21, p23, p25, p3, p5, p7, p17, p19, p9 : in  std_logic;
    p1, p11, p4, p2, p13, p15, p27, p28     : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6            : out std_logic;  -- test points
    p8, p18, p12, p10, p20                  : out std_logic;
    p6, p16, p14, p24, p26, p22             : out std_logic);

end qf;

architecture gates of qf is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component qfslice
    port (
      a, b, c    : in  std_logic;         -- inputs
      ca, cb, cc : in  std_logic;         -- controls
      tp         : out std_logic;         -- test point
      ya, yb, yc : out std_logic);        -- outputs
  end component;
  signal a, b, c, d, e, f : std_logic;
  signal t1, t2, t3, t4, t5, t6, t7 : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p4,
    y => a);
  u2 : inv port map (
    a => p2,
    y => b);
  u3 : inv port map (
    a => p13,
    y => c);
  u4 : inv port map (
    a => p15,
    y => d);
  u5 : inv port map (
    a => p27,
    y => e);
  u6 : inv port map (
    a => p28,
    y => f);
  u7 : qfslice port map (
    a  => p21,
    b  => p23,
    c  => p25,
    ca => a,
    cb => b,
    cc => c,
    tp => tp5,
    ya => p8,
    yb => t1,
    yc => t2);
  u8 : qfslice port map (
    a  => p3,
    b  => p5,
    c  => p7,
    ca => a,
    cb => b,
    cc => c,
    tp => tp6,
    ya => t3,
    yb => t4,
    yc => t5);
  u9 : qfslice port map (
    a  => p17,
    b  => p19,
    c  => p9,
    ca => a,
    cb => b,
    cc => c,
    tp => tp4,
    ya => t6,
    yb => t7,
    yc => p18);
  u10 : qfslice port map (
    a  => p1,
    b  => t1,
    c  => t3,
    ca => f,
    cb => d,
    cc => e,
    tp => tp1,
    ya => p12,
    yb => p10,
    yc => p20);
  u11 : qfslice port map (
    a  => t2,
    b  => t4,
    c  => t6,
    ca => f,
    cb => d,
    cc => e,
    tp => tp2,
    ya => p6,
    yb => p16,
    yc => p14);
  u12 : qfslice port map (
    a  => t5,
    b  => t7,
    c  => p11,
    ca => f,
    cb => d,
    cc => e,
    tp => tp3,
    ya => p24,
    yb => p26,
    yc => p22);

end gates;
