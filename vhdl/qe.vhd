-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- QE module, rev D -- shift control
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qeslice is
  
  port (
    a, b, c    : in  std_logic;
    ya, yb, yc : out std_logic);

end qeslice;

architecture qeslice of qeslice is
  component inv
    port (
      a  : in  std_logic;                     -- input
      y  : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1 : std_logic;
begin  -- qeslice

  u1 : inv port map (
    a => b,
    y => t1);
  yb <= t1;
  u2 : g2 port map (
    a => a,
    b => t1,
    y => ya);
  u3 : g2 port map (
    a => t1,
    b => c,
    y => yc);

end qeslice;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qe is
  
  port (
    p7, p11, p3, p4, p19              : in  std_logic;
    p23, p25, p12, p20, p24, p6, p10  : in  std_logic;
    tp1, tp2, tp5, tp6                : out std_logic;  -- test points
    p8, p2, p1, p5, p18, p16          : out std_logic;
    p26, p28, p21, p27, p17, p15, p13 : out std_logic;
    p22, p9, p14                      : out std_logic);

end qe;

architecture gates of qe is
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
  component latchd2
    port (
      d, d2, clk : in  std_logic;                 -- data (set), clock
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  component qeslice
    port (
      a, b, c    : in  std_logic;
      ya, yb, yc : out std_logic);
  end component;
  signal a, b, e, f : std_logic;
  signal t2, t3, t4 : std_logic;
begin  -- gates

  u1 : latch port map (
    d   => p7,
    d1  +. p11,
    clk => p10,
    q   => t2);
  u3 : inv port map (
    a => t2,
    y => a);
  tp2 <= a;
  u4 : g2 port map (
    a  => e,
    b  => t2,
    y2 => b);
  tp1 <= b;
  u5 : qeslice port map (
    a  => a,
    b  => p3,
    c  => b,
    ya => p8,
    yc => p2);
  u6 : qeslice port map (
    a  => a,
    b  => p4,
    c  => b,
    ya => p1,
    yc => p5);
  u7 : qeslice port map (
    a  => a,
    b  => p19,
    c  => b,
    ya => p18,
    yc => p16);
  u8 : qeslice port map (
    a  => a,
    b  => p23,
    c  => b,
    ya => p26,
    yc => p28);
  u9 : qeslice port map (
    a  => a,
    b  => p25,
    c  => b,
    ya => p21,
    yc => p27);
  u10 : qeslice port map (
    a  => a,
    b  => p12,
    c  => b,
    ya => p17,
    yb => t3,
    yc => p15);
  u11 : g2 port map (
    a => t3,
    b => f,
    y => p13);
  u12 : latch port map (
    d   => p20,
    d1  +. p24,
    clk => p10,
    q   => e);
  u13 : inv port map (
    a => e,
    y => f);
  tp5 <= f;
  u14 : inv2 port map (
    a  => p6,
    y  => tp6,
    y2 => t4);
  p22 <= t4;
  p9 <= t4;
  p14 <= t4;

end gates;
