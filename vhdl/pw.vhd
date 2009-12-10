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
-- PW module, rev B
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pw is
  
  port (
    p2, p4, p7, p5, p11, p9, p3                : in  logicsig;
    p22, p19, p17, p20, p26, p16, p10          : in  logicsig;
    tp1, tp5, tp6                              : out logicsig;  -- test points
    p8, p23, p21, p14, p12, p25, p27, p28, p24 : out logicsig);

end pw;

architecture gates of pw is
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
  component g3
    port (
      a, b, c : in  logicsig;                -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  logicsig;             -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  signal d, f, g, j, k, e : logicsig;
  signal t1, t2, t3, t4, t5, t6, t7, t8, t9, t10 : logicsig;
begin  -- gates

  u1 : inv2 port map (
    a  => p2,
    y  => f,
    y2 => g);
  u2 : inv2 port map (
    a  => p4,
    y  => j,
    y2 => k);
  u3 : inv port map (
    a => p7,
    y => e);
  u4 : g2 port map (
    a => p5,
    b => f,
    y => t1);
  u5 : g2 port map (
    a => g,
    b => p11,
    y => t2);
  u6 : g2 port map (
    a => p9,
    b => j,
    y => t3);
  u7 : g2 port map (
    a => k,
    b => p3,
    y => t4);
  u8 : g4 port map (
    a => t1,
    b => t2,
    c => t3,
    d => t4,
    y => t5);
  tp1 <= t5;
  u9 : g2 port map (
    a => t5,
    b => e,
    y => p8);
  u10 : g2 port map (
    a  => p22,
    b  => e,
    y  => p23,
    y2 => t6);
  u11 : inv port map (
    a => p17,
    y => d);
  u13 : g3 port map (
    a  => p19,
    b  => t6,
    c  => d,
    y  => t7,
    y2 => t8);
  tp5 <= t7;
  p21 <= t7;
  u14 : g2 port map (
    a => p16,
    b => t8,
    y => p14);
  u15 : g2 port map (
    a => t8,
    b => p10,
    y => p12);
  u16 : g2 port map (
    a  => p26,
    b  => e,
    y  => p24,
    y2 => t9);
  u17 : g3 port map (
    a => d,
    b => p20,
    c => t9,
    y => t10);
  tp6 <= t10;
  p27 <= t10;
  u18 : g2 port map (
    a  => t7,
    b  => t10,
    y  => p25,
    y2 => p28);
  
end gates;
