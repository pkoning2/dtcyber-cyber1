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
-- IP module -- 6 input gates
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ip is
  
  port (
    p4, p6, p8, p10, p12, p14    : in  std_logic;
    p1, p3, p5, p7, p9, p11      : in  std_logic;
    p17, p19, p21, p23, p25, p27 : in  std_logic;
    p16, p18, p20, p22, p24, p26 : in  std_logic;
    tp1, tp2, tp5, tp6           : out std_logic;  -- test points
    p13, p2, p28, p15            : out std_logic);

end ip;

architecture gates of ip is
  component g6
    port (
      a, b, c, d, e, f : in  std_logic;       -- inputs
      y, y2   : out std_logic);                  -- output
  end component;

begin  -- gates

  u1 : g6 port map (
    a  => p4,
    b  => p6,
    c  => p8,
    d  => p10,
    e  => p12,
    f  => p14,
    y  => tp1,
    y2 => p13);
  u2 : g6 port map (
    a  => p1,
    b  => p3,
    c  => p5,
    d  => p7,
    e  => p9,
    f  => p11,
    y  => tp2,
    y2 => p2);
  u3 : g6 port map (
    a  => p17,
    b  => p19,
    c  => p21,
    d  => p23,
    e  => p25,
    f  => p27,
    y  => tp5,
    y2 => p28);
  u4 : g6 port map (
    a  => p16,
    b  => p18,
    c  => p20,
    d  => p22,
    e  => p24,
    f  => p26,
    y  => tp6,
    y2 => p15);

end gates;
