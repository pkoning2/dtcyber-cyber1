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
-- QJ module -- 6 x coax latch, coax driver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qjslice is
  
  port (
    s    : in  coaxsig;
    r    : in  std_logic;
    q    : out std_logic;
    a, b : in  std_logic;
    tp   : out std_logic;               -- test point
    q2   : out coaxsig);

end qjslice;

architecture gates of qjslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  component cxdriver
    port (
      a : in  std_logic;                      -- source
      a2, a3, a4 : in std_logic := '1';   -- optional extra inputs
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
  signal t1, t2, t3 : std_logic;
begin  -- gates

  u1 : cxreceiver port map (
    a => s,
    y => t1);
  u2 : rsflop port map (
    s => t1,
    r => r,
    q => q);
  u3 : g2 port map (
    a => a,
    b => b,
    y => t3);
  tp <= t3;
  u4 : cxdriver port map (
    a => t3,
    y => q2);

end gates;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qj is
  
  port (
    p23, p22, p21, p10, p9, p8   : in  coaxsig;
    p11, p6, p4, p27, p25, p20   : in  std_logic;
    p18                          : in  std_logic;  -- clock/enable
    tp1, tp2, tp3, tp4, tp5, tp6 : out std_logic;  -- test
    p19, p17, p15, p16, p14, p12 : out std_logic;
    p7, p5, p3, p28, p26, p24    : out coaxsig);

end qj;

architecture gates of qj is
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
  component qjslice
    port (
      s    : in  coaxsig;
      r    : in  std_logic;
      q    : out std_logic;
      a, b : in  std_logic;
      tp   : out std_logic;               -- test point
      q2   : out coaxsig);
  end component;
  signal t1, a, c : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p18,
    y2 => c);
  u2 : inv2 port map (
    a  => c,
    y2 => t1);
  u3 : inv port map (
    a => t1,
    y => a);
  u4 : qjslice port map (
    s  => p23,
    r  => a,
    q  => p19,
    a  => p11,
    b  => c,
    tp => tp1,
    q2 => p7);
  u5 : qjslice port map (
    s  => p22,
    r  => a,
    q  => p17,
    a  => p6,
    b  => c,
    tp => tp2,
    q2 => p5);
  u6 : qjslice port map (
    s  => p21,
    r  => a,
    q  => p15,
    a  => p4,
    b  => c,
    tp => tp3,
    q2 => p3);
  u7 : qjslice port map (
    s  => p10,
    r  => a,
    q  => p16,
    a  => p27,
    b  => c,
    tp => tp4,
    q2 => p28);
  u8 : qjslice port map (
    s  => p9,
    r  => a,
    q  => p14,
    a  => p25,
    b  => c,
    tp => tp5,
    q2 => p26);
  u9 : qjslice port map (
    s  => p8,
    r  => a,
    q  => p12,
    a  => p20,
    b  => c,
    tp => tp6,
    q2 => p24);
  
end gates;
