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
-- PI module rev C
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pislice is
  
  port (
    s     : in  coaxsig;
    r     : in  std_logic;
    tp    : out std_logic;              -- test point
    q, qb : out std_logic);

end pislice;

architecture gates of pislice is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
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
  signal t1, t2 : std_logic;
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
    y2 => q);

end gates;
library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pi is
  
  port (
    p5, p10, p24, p7, p21, p26  : in  coaxsig;
    p16, p15                    : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p9, p12, p13, p14, p28, p27 : out std_logic;
    p1, p4, p18, p17, p22, p19  : out std_logic);

end pi;

architecture gates of pi is
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
  component pislice
    port (
      s     : in  coaxsig;
      r     : in  std_logic;
      tp    : out std_logic;              -- test point
      q, qb : out std_logic);
  end component;
  signal a, t1 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a  => p16,
    b  => p15,
    y2 => t1);
  u9 : inv port map (
    a => t1,
    y => a);
  u2 : pislice port map (
    s  => p5,
    r  => a,
    tp => tp1,
    q  => p9,
    qb => p12);
  u3 : pislice port map (
    s  => p7,
    r  => a,
    tp => tp2,
    q  => p1,
    qb => p4);
  u4 : pislice port map (
    s  => p10,
    r  => a,
    tp => tp3,
    q  => p13,
    qb => p14);
  u5 : pislice port map (
    s  => p21,
    r  => a,
    tp => tp4,
    q  => p18,
    qb => p17);
  u6 : pislice port map (
    s  => p24,
    r  => a,
    tp => tp5,
    q  => p28,
    qb => p27);
  u7 : pislice port map (
    s  => p26,
    r  => a,
    tp => tp6,
    q  => p22,
    qb => p19);
  
end gates;
