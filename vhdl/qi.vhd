-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QI module -- I/O cable transceiver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qislice is
  
  port (
    s          : in  coaxsig;           -- input cable
    r, e       : in  std_logic;         -- reset, enable
    q1, q2, qb : out std_logic;         -- latch outputs
    q          : out coaxsig;           -- output cable
    tp         : out std_logic);        -- test point

end qislice;

architecture gates of qislice is
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
  component cxdriver
    port (
      a : in  std_logic;                      -- source
      y : out coaxsig);                       -- destination
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
  signal t1, t2, t3, t4 : std_logic;
begin  -- gates

  u1 : cxreceiver port map (
    a => s,
    y => t1);
  u2 : rsflop port map (
    s => t1,
    r => r,
    q => t2);
  tp <= t2;
  u3 : inv2 port map (
    a  => t2,
    y  => qb,
    y2 => t3);
  q1 <= t3;
  q2 <= t3;
  u4 : g2 port map (
    a => t3,
    b => e,
    y => t4);
  u5 : cxdriver port map (
    a => t4,
    y => q);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qi is
  
  port (
    p3, p13, p16, p26            : in  coaxsig;    -- input cables
    p12                          : in  std_logic;  -- clear/enable
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p7, p5, p6, p9, p11, p8      : out std_logic;
    p20, p18, p23, p22, p24, p25 : out std_logic;
    p1, p15, p14, p28            : out coaxsig);   -- output cable

end qi;

architecture gates of qi is
  component qislice
    port (
      s          : in  coaxsig;           -- input cable
      r, e       : in  std_logic;         -- reset, enable
      q1, q2, qb : out std_logic;         -- latch outputs
      q          : out coaxsig;           -- output cable
      tp         : out std_logic);        -- test point
  end component;
    component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  signal a, b, t1, t2 : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p12,
    y  => t1,
    y2 => b);
  u2 : inv2 port map (
    a  => t1,
    y2 => t2);
  u3 : inv2 port map (
    a => t2,
    y => a);
  u4 : qislice port map (
    s  => p3,
    r  => a,
    e  => b,
    q1 => p7,
    q2 => p5,
    qb => p6,
    q  => p1,
    tp => tp1);
  u5 : qislice port map (
    s  => p13,
    r  => a,
    e  => b,
    q1 => p9,
    q2 => p11,
    qb => p8,
    q  => p15,
    tp => tp2);
  u6 : qislice port map (
    s  => p16,
    r  => a,
    e  => b,
    q1 => p20,
    q2 => p18,
    qb => p23,
    q  => p14,
    tp => tp5);
  u7 : qislice port map (
    s  => p26,
    r  => a,
    e  => b,
    q1 => p22,
    q2 => p24,
    qb => p25,
    q  => p28,
    tp => tp6);

end gates;
