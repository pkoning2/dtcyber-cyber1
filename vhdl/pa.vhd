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
-- PA module rev C -- memory address register (S register)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity paslice is
  
  port (
    d, clk, clk2 : in  logicsig;             -- data and clocks
    tp     : out logicsig;             -- test point
    q1, q2 : out logicsig);            -- outputs

end paslice;

architecture gates of paslice is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2  : out logicsig);                    -- output
  end component;
  component latch2
    port (
      d, clk, clk2 : in  logicsig;                 -- data (set), clocks
      q, qb  : out logicsig);                -- q and q.bar
  end component;
  signal tq, tq2 : logicsig;                 -- latch output
begin  -- gates
  u1 : latch2 port map (
    d   => d,
    clk => clk,
    clk2 => clk2,
    q   => tq);
  tp <= tq;
  u2 : inv2 port map (
    a => tq,
    y2 => tq2);
  q1 <= tq2;
  q2 <= tq2;
end gates;

use work.sigs.all;

entity pa is
  
  port (
    p8, p5, p12, p21, p24, p23   : in  logicsig;   -- inputs
    p9, p7, p20, p22             : in  logicsig;   -- strobes
    tp1, tp2, tp3, tp4, tp5, tp6 : out logicsig;   -- test points
    p1, p3, p2, p6, p14, p10     : out logicsig;   -- outputs
    p19, p17, p27, p25, p28, p26 : out logicsig);  -- more outputs

end pa;

architecture gates of pa is
  component paslice
    port (
      d, clk, clk2 : in  logicsig;           -- data and clocks
      tp     : out logicsig;           -- test point
      q1, q2 : out logicsig);          -- outputs
  end component;
begin  -- gates
  u5 : paslice port map (
    d   => p8,
    clk => p9,
    clk2 => p7,
    tp  => tp1,
    q1  => p1,
    q2  => p3);
  u6 : paslice port map (
    d   => p5,
    clk => p9,
    clk2 => p7,
    tp  => tp2,
    q1  => p2,
    q2  => p6);
  u7 : paslice port map (
    d   => p12,
    clk => p9,
    clk2 => p7,
    tp  => tp3,
    q1  => p14,
    q2  => p10);
  u8 : paslice port map (
    d   => p21,
    clk => p20,
    clk2 => p22,
    tp  => tp4,
    q1  => p19,
    q2  => p17);
  u9 : paslice port map (
    d   => p24,
    clk => p20,
    clk2 => p22,
    tp  => tp5,
    q1  => p27,
    q2  => p25);
  u10 : paslice port map (
    d   => p23,
    clk => p20,
    clk2 => p22,
    tp  => tp6,
    q1  => p28,
    q2  => p26);

end gates;
