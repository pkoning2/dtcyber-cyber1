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
-- PN module, rev C
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pnslice is
  
  port (
    s, r  : in  logicsig;            -- inputs
    a, b  : in  logicsig;            -- enables
    c     : in  logicsig;            -- reset
    tp    : out logicsig;            -- test point
    q, q2 : out logicsig);           -- outputs

end pnslice;

architecture gates of pnslice is
  component inv
    port (
      a  : in  logicsig;                     -- input
      y  : out logicsig);                    -- output
  end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component r2sflop
    port (
      s, r, r2  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal tsi, tri, tqbi, tqi : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => s,
    b => a,
    y => tsi);
  u2 : g2 port map (
    a => r,
    b => b,
    y => tri);
  u3 : r2sflop port map (
    s  => tsi,
    r  => tri,
    r2 => c,
    q  => tp,
    qb => tqbi);
  u4 : inv port map (
    a => tqbi,
    y => tqi);
  q <= tqi;
  q2 <= tqi;
  
end gates;


use work.sigs.all;

entity pn is
  
  port (
    p2, p3, p1, p4, p28, p25, p27, p26                : in  logicsig;
    p20, p18, p17                                     : in  logicsig;
    tp1, tp2, tp5, tp6 : out logicsig;  -- test points
    p15, p5, p12, p8, p21, p19, p24, p22 : out logicsig);

end pn;

architecture gates of pn is
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component pnslice
    port (
      s, r  : in  logicsig;            -- inputs
      a, b  : in  logicsig;            -- enables
      c     : in  logicsig;            -- reset
      tp    : out logicsig;            -- test point
      q, q2 : out logicsig);           -- outputs
  end component;
  signal a, b, c : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => p20,
    y2 => a);
  u2 : inv2 port map (
    a  => p18,
    y2 => b);
  u3 : inv2 port map (
    a  => p17,
    y2 => c);
  u4 : pnslice port map (
    s  => p2,
    r  => p3,
    a  => a,
    b  => b,
    c  => c,
    tp => tp1,
    q  => p15,
    q2 => p5);
  u5 : pnslice port map (
    s  => p1,
    r  => p4,
    a  => a,
    b  => b,
    c  => c,
    tp => tp2,
    q  => p12,
    q2 => p8);
  u6 : pnslice port map (
    s  => p28,
    r  => p25,
    a  => a,
    b  => b,
    c  => c,
    tp => tp5,
    q  => p21,
    q2 => p19);
  u7 : pnslice port map (
    s  => p27,
    r  => p26,
    a  => a,
    b  => b,
    c  => c,
    tp => tp6,
    q  => p24,
    q2 => p22);

end gates;
