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
-- PL module rev E - coax drivers
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity plslice is
  
  port (
    a, b : in  std_logic;
    tp   : out std_logic;               -- test point
    y    : out coaxsig);

end plslice;

architecture gates of plslice is
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
  signal t : std_logic;
begin  -- gates

  u1 : g2 port map (
    a  => a,
    b  => b,
    y  => t,
    y2 => tp);
  u2 : cxdriver port map (
    a => t,
    y => y);

end gates;
library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity pl is
  
  port (
    p9, p13, p20, p11, p18, p22 : in  std_logic;
    p10, p21                    : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test points
    p8, p4, p25, p6, p27, p23   : out coaxsig);

end pl;

architecture gates of pl is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component plslice
    port (
      a, b : in  std_logic;
      tp   : out std_logic;               -- test point
      y    : out coaxsig);
  end component;
  signal a : std_logic;
begin  -- gates

  u1 : g2 port map (
    a  => p21,
    b  => p10,
    y2 => a);
  u2 : plslice port map (
    a  => p9,
    b  => a,
    tp => tp1,
    y  => p8);
  u3 : plslice port map (
    a  => p11,
    b  => a,
    tp => tp2,
    y  => p6);
  u4 : plslice port map (
    a  => p13,
    b  => a,
    tp => tp3,
    y  => p4);
  u5 : plslice port map (
    a  => p18,
    b  => a,
    tp => tp4,
    y  => p27);
  u6 : plslice port map (
    a  => p20,
    b  => a,
    tp => tp5,
    y  => p25);
  u7 : plslice port map (
    a  => p22,
    b  => a,
    tp => tp6,
    y  => p23);

end gates;
