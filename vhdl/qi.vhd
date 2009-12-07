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
-- QI module -- I/O cable transceiver
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qislice is
  
  port (
    s          : in  coaxsig;           -- input cable
    r, e       : in  logicsig;         -- reset, enable
    q1, q2, qb : out logicsig;         -- latch outputs
    q          : out coaxsig;           -- output cable
    tp         : out logicsig);        -- test point

end qislice;

architecture gates of qislice is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component cxdriver
    port (
      a : in  logicsig;                      -- source
      y : out coaxsig);                       -- destination
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                        -- source
      y : out logicsig);                     -- destination
  end component;
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal t1, t2, t3, t4 : logicsig;
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
    p12                          : in  logicsig;  -- clear/enable
    tp1, tp2, tp5, tp6           : out logicsig;  -- test points
    p7, p5, p6, p9, p11, p8      : out logicsig;
    p20, p18, p23, p22, p24, p25 : out logicsig;
    p1, p15, p14, p28            : out coaxsig);   -- output cable

end qi;

architecture gates of qi is
  component qislice
    port (
      s          : in  coaxsig;           -- input cable
      r, e       : in  logicsig;         -- reset, enable
      q1, q2, qb : out logicsig;         -- latch outputs
      q          : out coaxsig;           -- output cable
      tp         : out logicsig);        -- test point
  end component;
    component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  signal a, b, t1, t2 : logicsig;
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
    y2 => a);
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
