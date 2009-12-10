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
-- AD module - 6612 S register
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity adslice is
  
  port (
    s, e, r     : in  logicsig;        -- set, enable, reset
    tp          : out logicsig;        -- test point
    q, qb1, qb2 : out logicsig);       -- outputs

end adslice;

architecture gates of adslice is
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
  component rsflop
    port (
      s, r  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal t1, t2, t3, t4 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => s,
    b => e,
    y => t1);
  u2 : rsflop port map (
    s  => t1,
    r  => r,
    q  => t2,
    qb => t3);
  q <= t2;
  tp <= t2;
  u3 : inv2 port map (
    a => t3,
    y => t4);
  qb1 <= t4;
  qb2 <= t4;
  
end gates;

use work.sigs.all;

entity ad is
  
  port (
    p18, p5, p25, p4, p24, p11               : in  logicsig;
    p16, p15, p13, p14                       : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6             : out logicsig;  -- test points
    p17, p19, p20, p6, p8, p7, p26, p28, p27 : out logicsig;
    p3, p1, p2, p23, p21, p22, p12, p10, p9  : out logicsig);

end ad;

architecture gates of ad is
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
  component adslice
    port (
      s, e, r     : in  logicsig;        -- set, enable, reset
      tp          : out logicsig;        -- test point
      q, qb1, qb2 : out logicsig);       -- outputs
  end component;
  signal a, c, d : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => p16,
    y2 => a);
  u2 : g2 port map (
    a  => p15,
    b  => p13,
    y2 => c);
  u3 : g2 port map (
    a  => p13,
    b  => p14,
    y2 => d);
  u4 : adslice port map (
    s   => p18,
    e   => a,
    r   => c,
    tp  => tp4,
    q   => p17,
    qb1 => p19,
    qb2 => p20);
  u5 : adslice port map (
    s   => p5,
    e   => a,
    r   => d,
    tp  => tp1,
    q   => p6,
    qb1 => p8,
    qb2 => p7);
  u6 : adslice port map (
    s   => p25,
    e   => a,
    r   => c,
    tp  => tp5,
    q   => p26,
    qb1 => p28,
    qb2 => p27);
  u7 : adslice port map (
    s   => p4,
    e   => a,
    r   => d,
    tp  => tp2,
    q   => p3,
    qb1 => p1,
    qb2 => p2);
  u8 : adslice port map (
    s   => p24,
    e   => a,
    r   => c,
    tp  => tp6,
    q   => p23,
    qb1 => p21,
    qb2 => p22);
  u9 : adslice port map (
    s   => p11,
    e   => a,
    r   => d,
    tp  => tp3,
    q   => p12,
    qb1 => p10,
    qb2 => p9);

end gates;
