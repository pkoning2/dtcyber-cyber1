-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- QC module, rev B
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qcslice is
  
  port (
    a, b, c : in  std_logic;
    d, e    : in  std_logic := '1';     -- optional inputs
    tp      : out std_logic;            -- test point
    y       : out std_logic;            -- output
    ya      : out std_logic;            -- nor gate output (matches test point)
    yb      : out std_logic);           -- bottom output

end qcslice;

architecture gates of qcslice is
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal t1, t2, t3 : std_logic;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);
  u2 : g2 port map (
    a => c,
    b => d,
    y => t2);
  yb <= t2;
  u3 : g2 port map (
    a => t1,
    b => t2,
    y => t3);
  tp <= t3;
  ya <= t3;
  u4 : g2 port map (
    a => t3,
    b => e,
    y => y);
  
end gates;


library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity qc is
  
  port (
    p6, p4, p8, p15, p19, p25, p27, p23        : in  std_logic;
    p5, p3, p14, p16, p20, p26, p28            : in  std_logic;
    tp1, tp2, tp3, tp4, tp5, tp6               : out std_logic;  -- test points
    p10, p17, p21, p7, p11, p24, p12, p13, p18 : out std_logic);

end qc;

architecture gates of qc is
  component qcslice
    port (
      a, b, c : in  std_logic;
      d, e    : in  std_logic := '1';     -- optional inputs
      tp      : out std_logic;            -- test point
      y       : out std_logic;            -- output
      ya      : out std_logic;            -- nor gate output (matches test point)
      yb      : out std_logic);           -- bottom output
  end component;
  signal a, b, c, d, e, f : std_logic;
begin  -- gates

  u1 : qcslice port map (
    a  => p6,
    b  => p4,
    c  => p8,
    tp => tp1,
    y  => p10,
    ya => a);
  u2 : qcslice port map (
    a  => p5,
    b  => p3,
    c  => a,
    d  => f,
    tp => tp2,
    y  => p7,
    ya => e,
    yb => p13,
    e  => a);
  u3 : qcslice port map (
    a  => p15,
    b  => p19,
    c  => b,
    d  => d,
    tp => tp3,
    y  => p17,
    ya => f,
    yb => p12,
    e  => b);
  u4 : qcslice port map (
    a  => p14,
    b  => p16,
    c  => p20,
    tp => tp4,
    y  => p11,
    ya => b);
  u5 : qcslice port map (
    a  => p25,
    b  => p27,
    c  => p23,
    tp => tp5,
    y  => p21,
    ya => c);
  u6 : qcslice port map (
    a  => p26,
    b  => p28,
    c  => c,
    d  => e,
    tp => tp6,
    y  => p24,
    ya => d,
    yb => p18,
    e  => c);
  
end gates;
