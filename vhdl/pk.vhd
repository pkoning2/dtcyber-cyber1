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
-- PK module rev D -- channel control
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pk is
  
  port (
    p19 : in  std_logic;               -- select channel
    p15 : in  coaxsig := '1';                 -- coax full input
    p6  : in  std_logic;               -- full
    p17 : in  std_logic;               -- empty
    p8  : in  coaxsig := '1';                 -- coax empty input
    p23 : in  std_logic;               -- master clear
    p5  : in  std_logic;               -- function
    p3  : out coaxsig;                        -- function coax output
    p9  : out coaxsig;                        -- empty coax output
    p10 : out std_logic;                      -- empty out
    p18 : out coaxsig;                        -- inactive coax out
    p14 : in  coaxsig := '1';                 -- inactive coax in
    p21 : out std_logic;                      -- inactive out
    p25 : in  coaxsig := '1';                 -- active coax in
    p22 : out coaxsig;                        -- active coax out
    p12 : in  std_logic;               -- active in
    p27 : in  std_logic;               -- inactive in
    p24, p26, p28 : out std_logic;            -- empty out
    p13 : out coaxsig;                        -- coax full out
    tp1 : out std_logic;                      -- test point 1
    tp2 : out std_logic;                      -- test point 2
    tp5 : out std_logic;                      -- test point 5
    tp6 : out std_logic);                     -- test point 6

end pk;

architecture gates of pk is
  component inv
    port (
      a : in  std_logic;                      -- input
      y : out std_logic);                     -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2    : out std_logic);                  -- output
  end component;
  component cxdriver
    port (
      a : in  std_logic;                        -- source
      a2, a3, a4 : in std_logic := '1';   -- optional extra inputs
      y : out coaxsig);                       -- destination
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                    -- source
      y : out std_logic);                 -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- inputs
      s2, s3, s4, r2, r3, r4  : in  std_logic;  -- extra set, reset if needed
      q, qb : out std_logic);                 -- outputs
  end component;
  signal b, c : std_logic;                    -- internal enables
  signal ai, ii, fi, ei : std_logic;          -- internal gate output terms
  signal fq, f2 : std_logic;                  -- full rsflop outputs
  signal a : std_logic;                       -- active rsflop output
  signal p15i, p8i, p25i, p14i : std_logic;   -- internal coax inputs
begin  -- gates

  u1 : inv port map (
    a => p19,
    y => c);
  tp6 <= c;
  u2 : g2 port map (
    a => p6,
    b => c,
    y => fi);
  u3 : g2 port map (
    a => c,
    b => p17,
    y => ei);
  u4 : g2 port map (
    a => p5,
    b => c,
    y => b);
  tp1 <= b;
  u5 : g2 port map (
    a => p12,
    b => c,
    y => ai);
  u6 : g2 port map (
    a => p27,
    b => c,
    y => ii);
  u7 : cxdriver port map (
    a => fi,
    y => p13);
  u8 : cxdriver port map (
    a => ei,
    y => p9);
  u9 : cxdriver port map (
    a => b,
    y => p3);
  u10 : cxdriver port map (
    a => ai,
    y => p22);
  u11 : cxdriver port map (
    a => ii,
    y => p18);
  u12 : cxreceiver port map (
    a => p15,
    y => p15i);
  u13 : cxreceiver port map (
    a => p8,
    y => p8i);
  u14 : rsflop port map (
    s  => fi,
    s2 => p15i,
    s3 => b,
    r  => ei,
    r2 => p8i,
    r3 => a,
    r4 => p23,
    q  => fq,
    qb => p10);
  tp2 <= fq;
  u15 : inv port map (
    a => fq,
    y => f2);
  p28 <= f2;
  p26 <= f2;
  p24 <= f2;
  u18 : cxreceiver port map (
    a => p25,
    y => p25i);
  u19 : cxreceiver port map (
    a => p14,
    y => p14i);
  u20 : rsflop port map (
    s  => ai,
    s2 => p23,
    s3 => b,
    s4 => p25i,
    r  => ii,
    r2 => p14i,
    q  => a,
    qb => p21);
  tp5 <= a;
  
end gates;
