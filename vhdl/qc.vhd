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
-- QC module, rev B
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity qcslice1 is
  
  port (
    a, b, c : in  logicsig;
    tp      : out logicsig;            -- test point
    y       : out logicsig;            -- output
    ya      : out logicsig);           -- nor gate output (matches test point)

end qcslice1;

architecture gates of qcslice1 is
  component inv is
      port (
          a  : in  logicsig;                       -- input
          y  : out logicsig);                      -- output
    end component;
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2, t3 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => b,
    y => t1);
  u2 : inv port map (
    a => c,
    y => t2);
  u3 : g2 port map (
    a => t1,
    b => t2,
    y => t3,
    y2 => y);
  tp <= t3;
  ya <= t3;
  
end gates;


use work.sigs.all;

entity qcslice2 is
  
  port (
    a, b, c, d, e : in  logicsig;
    tp      : out logicsig;            -- test point
    y       : out logicsig;            -- output
    ya      : out logicsig;            -- inverter output (matches test point)
    yb      : out logicsig);           -- bottom output

end qcslice2;

architecture gates of qcslice2 is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g3
    port (
      a, b, c : in  logicsig;                   -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal t1, t2 : logicsig;
begin  -- gates

  u1 : g2 port map (
    a => a,
    b => b,
    y2 => t1);
  tp <= t1;
  ya <= t1;
  u2 : g3 port map (
    a => t1,
    b => c,
    c => d,
    y => t2);
  yb <= t2;
  u4 : g2 port map (
    a => t1,
    b => e,
    y => y);
  
end gates;


use work.sigs.all;

entity qc is
  
  port (
    p6, p4, p8, p15, p19, p25, p27, p23        : in  logicsig;
    p5, p3, p14, p16, p20, p26, p28            : in  logicsig;
    tp1, tp2, tp3, tp4, tp5, tp6               : out logicsig;  -- test points
    p10, p17, p21, p7, p11, p24, p12, p13, p18 : out logicsig);

end qc;

architecture gates of qc is
  component qcslice1
    port (
      a, b, c : in  logicsig;
      tp      : out logicsig;            -- test point
      y       : out logicsig;            -- output
      ya      : out logicsig);           -- nor gate output (matches test point)
  end component;
  component qcslice2
    port (
      a, b, c, d, e : in  logicsig;
      tp      : out logicsig;            -- test point
      y       : out logicsig;            -- output
      ya      : out logicsig;            -- inverter output (matches test point)
      yb      : out logicsig);           -- bottom output
  end component;
  signal a, b, c, d, e, f : logicsig;
begin  -- gates

  u1 : qcslice1 port map (
    a  => p6,
    b  => p4,
    c  => p8,
    tp => tp1,
    y  => p10,
    ya => a);
  u2 : qcslice2 port map (
    a  => p5,
    b  => p3,
    c  => a,
    d  => f,
    e  => a,
    tp => tp2,
    y  => p7,
    ya => e,
    yb => p13);
  u3 : qcslice2 port map (
    a  => p15,
    b  => p19,
    c  => b,
    d  => d,
    e  => b,
    tp => tp3,
    y  => p17,
    ya => f,
    yb => p12);
  u4 : qcslice1 port map (
    a  => p14,
    b  => p16,
    c  => p20,
    tp => tp4,
    y  => p11,
    ya => b);
  u5 : qcslice1 port map (
    a  => p25,
    b  => p27,
    c  => p23,
    tp => tp5,
    y  => p21,
    ya => c);
  u6 : qcslice2 port map (
    a  => p26,
    b  => p28,
    c  => c,
    d  => e,
    e  => c,
    tp => tp6,
    y  => p24,
    ya => d,
    yb => p18);
  
end gates;
