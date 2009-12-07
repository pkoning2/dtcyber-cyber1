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
-- PS module, rev H -- memory sense amplifier
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity psslice is
  
  port (
    a  : in  logicsig;
    tp : out logicsig;                 -- test point
    b  : in  logicsig;                 -- enable
    y  : out logicsig);

end psslice;

architecture gates of psslice is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
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
  signal t1, t2 : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => a,
    y2 => t1);
  tp <= t1;
  u2 : inv port map (
    a => t1,
    y => t2);
  u3 : g2 port map (
    a => t2,
    b => b,
    y => y);

end gates;

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity ps is
  
  port (
    p2, p4, p8, p10, p16, p18, p22, p24, p27 : in    logicsig;
    tp1, tp2, tp3, tp4, tp6                  : out   logicsig;  -- test points
    p5, p11, p19, p25                        : out   logicsig;
    p28, p26                                 : inout misc);      -- bias

end ps;

-- For simplicity we only use the true (not the complemented) inputs.
-- There's not much sense in doing "differential" processing inside
-- an FPGA...

architecture gates of ps is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component psslice
    port (
      a  : in  logicsig;
      tp : out logicsig;                 -- test point
      b  : in  logicsig;                 -- enable
      y  : out logicsig);
  end component;
  signal a : logicsig;
begin  -- gates
  u1 : inv2 port map (
    a  => p27,
    y2 => a);
  tp6 <= a;
  u2 : psslice port map (
    a  => p2,
    tp => tp1,
    b  => a,
    y  => p5);
  u3 : psslice port map (
    a  => p8,
    tp => tp2,
    b  => a,
    y  => p11);
  u4 : psslice port map (
    a  => p16,
    tp => tp3,
    b  => a,
    y  => p19);
  u5 : psslice port map (
    a  => p22,
    tp => tp4,
    b  => a,
    y  => p25);

end gates;
