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
-- TJ module, rev B - two port mux
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tjslice is
  
  port (
    a      : in  logicsig;             -- select
    b, c   : in  logicsig;             -- inputs
    tp     : out logicsig;             -- test point
    y1, y2 : out logicsig);            -- outputs

end tjslice;

architecture gates of tjslice is
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
  signal t1, t2, t3, t4, t5 : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => a,
    y  => t1,
    y2 => t2);
  tp <= t2;
  u2 : g2 port map (
    a => t1,
    b => b,
    y => t3);
  u3 : g2 port map (
    a => t2,
    b => c,
    y => t4);
  u4 : g2 port map (
    a => t3,
    b => t4,
    y => t5);
  y1 <= t5;
  y2 <= t5;
  
end gates;

use work.sigs.all;

entity tj is
  
  port (
    p10, p9, p8, p7, p6, p5      : in  logicsig;
    p24, p25, p26, p21, p22, p23 : in  logicsig;
    tp1, tp2, tp5, tp6           : out logicsig;  -- test points
    p12, p14, p4, p11            : out logicsig;
    p27, p20, p19, p17           : out logicsig);

end tj;

architecture gates of tj is
  component tjslice
    port (
      a      : in  logicsig;             -- select
      b, c   : in  logicsig;             -- inputs
      tp     : out logicsig;             -- test point
      y1, y2 : out logicsig);            -- outputs
  end component;
begin  -- gates

  u1 : tjslice port map (
    a  => p10,
    b  => p9,
    c  => p8,
    tp => tp1,
    y1 => p12,
    y2 => p14);
  u2 : tjslice port map (
    a  => p7,
    b  => p6,
    c  => p5,
    tp => tp2,
    y1 => p4,
    y2 => p11);
  u3 : tjslice port map (
    a  => p24,
    b  => p25,
    c  => p26,
    tp => tp5,
    y1 => p27,
    y2 => p20);
  u4 : tjslice port map (
    a  => p21,
    b  => p22,
    c  => p23,
    tp => tp6,
    y1 => p19,
    y2 => p17);
  
end gates;
