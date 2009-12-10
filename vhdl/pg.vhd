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
-- PG modulerev C  - storage address register (G)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pgslice is
  
  port (
    i1, i2     : in  logicsig;               -- input bits
    a          : in  logicsig;               -- reset
    b, d, e    : in  logicsig;               -- common enables/clocks
    tp         : out logicsig;               -- test point
    q          : out logicsig);              -- output

end pgslice;

architecture gates of pgslice is
  component inv
    port (
      a : in  logicsig;                      -- input
      y : out logicsig);                     -- output
  end component;
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2       : out logicsig);               -- output
  end component;
  component rs2flop
    port (
      s, s2, r  : in  logicsig;                  -- inputs
      q, qb : out logicsig);                 -- outputs
  end component;
  signal t1, t2, ti : logicsig;           -- intermediate values
begin  -- gates

  u1 : g3 port map (
    a => e,
    b => i1,
    c => b,
    y => t1);
  u2 : g3 port map (
    a => b,
    b => i2,
    c => d,
    y => t2);
  u3 : rs2flop port map (
    s  => t1,
    s2 => t2,
    r  => a,
    q  => ti);
  tp <= ti;
  u4 : inv port map (
    a => ti,
    y => q);
  
end gates;

use work.sigs.all;

entity pg is

  port (
    p10, p8, p9, p21, p7, p23 : in  logicsig;  -- enables
    p5, p3, p6, p4            : in  logicsig;  -- inputs 1 and 2
    p25, p27, p26, p28        : in  logicsig;  -- inputs 3 and 4
    p13, p14, p17, p20        : out logicsig;  -- outputs
    tp1, tp2, tp5, tp6        : out logicsig);  -- test points

end pg;

architecture gates of pg is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2    : out logicsig);                  -- output
  end component;
  component inv
    port (
      a : in  logicsig;                      -- input
      y : out logicsig);                     -- output
  end component;
  component inv2
    port (
      a : in  logicsig;                      -- input
      y, y2 : out logicsig);                     -- output
  end component;
  component pgslice
    port (
      i1, i2     : in  logicsig;               -- input bits
      a, b, d, e : in  logicsig;               -- common enables/clocks
      tp         : out logicsig;               -- test point
      q          : out logicsig);              -- output
  end component;
  signal a, b, d, e : logicsig;          -- control signals
begin  -- gates

  u1 : inv port map (
    a => p10,
    y => a);
  u2 : inv2 port map (
    a => p8,
    y2 => b);
  u4 : g2 port map (
    a => p9,
    b => p21,
    y => d);
  u5 : g2 port map (
    a => p7,
    b => p23,
    y => e);
  u6 : pgslice port map (
    i1 => p5,
    i2 => p3,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp1,
    q  => p13);
  u7 : pgslice port map (
    i1 => p6,
    i2 => p4,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp2,
    q  => p14);
  u8 : pgslice port map (
    i1 => p25,
    i2 => p27,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp5,
    q  => p17);
  u9 : pgslice port map (
    i1 => p26,
    i2 => p28,
    a  => a,
    b  => b,
    d  => d,
    e  => e,
    tp => tp6,
    q  => p20);

end gates;

