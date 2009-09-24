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
-- HQ module -- clock receiver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity hq is
  
  port (
    p12                                    : in  coaxsig;  -- master clock input
    p10                                    : in  std_logic;  -- clock latch reset
    p16, p13, p15, p19                     : in  std_logic;  -- clock I, II, III, IV
    p11, p18, p17, p20                     : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6           : out std_logic;  -- test points
    p14                                    : out std_logic;
    p2, p4, p6, p8, p1, p3, p5, p7         : out std_logic;
    p22, p24, p26, p28, p21, p23, p25, p27 : out std_logic);

end hq;

architecture gates of hq is
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
  component cxreceiver
    port (
      a : in  coaxsig;                        -- source
      y : out std_logic);                     -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      q, qb : out std_logic);                 -- q and q.bar
  end component;
  signal t1, t2, t3, t4, t5, t6 : std_logic;
  signal t7, t8, t9, t10, t11 : std_logic;
begin  -- gates

  u1 : inv port map (
    a => p10,
    y => t1);
  tp4 <= t1;
  u2 : cxreceiver port map (
    a => p12,
    y => t2);
  u3 : rsflop port map (
    s => t2,
    r => t1,
    q => t3);
  tp1 <= t3;
  p14 <= t3;
  u4 : inv port map (
    a => p16,
    y => t4);
  u5 : g2 port map (
    a  => t4,
    b  => p11,
    y  => tp2,
    y2 => t5);
  p2 <= t5;
  p4 <= t5;
  p6 <= t5;
  p8 <= t5;
  u6 : inv port map (
    a => p13,
    y => t6);
  u7 : g2 port map (
    a  => t6,
    b  => p18,
    y  => tp3,
    y2 => t7);
  p1 <= t7;
  p3 <= t7;
  p5 <= t7;
  p7 <= t7;
  u8 : inv port map (
    a => p15,
    y => t8);
  u9 : g2 port map (
    a  => t8,
    b  => p17,
    y  => tp5,
    y2 => t9);
  p22 <= t9;
  p24 <= t9;
  p26 <= t9;
  p28 <= t9;
  u10 : inv port map (
    a => p19,
    y => t10);
  u11 : g2 port map (
    a  => t10,
    b  => p20,
    y  => tp6,
    y2 => t11);
  p21 <= t11;
  p23 <= t11;
  p25 <= t11;
  p27 <= t11;
  
end gates;
