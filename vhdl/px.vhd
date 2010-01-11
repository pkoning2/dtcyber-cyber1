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
-- PX module, rev D
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity px is
  
  port (
    p11                          : in  coaxsig;
    p7, p2, p5, p9, p6, p12, p4  : in  logicsig;
    p22, p10, p23, p24, p21, p26 : in  logicsig;
    tp1, tp2, tp5, tp6           : out logicsig;  -- test points
    p13, p28, p18, p17, p20      : out logicsig;
    p15, p14, p16, p3, p1        : out logicsig;
    p8, p27, p25, p19            : out logicsig);

end px;

architecture gates of px is
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
  component g5
    port (
      a, b, c, d, e : in  logicsig;          -- inputs
      y, y2   : out logicsig);                  -- output
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                        -- source
      y : out logicsig);                     -- destination
  end component;
  component r2sflop
    port (
      s, r, r2  : in  logicsig;                  -- set, reset
      q, qb : out logicsig);                 -- q and q.bar
  end component;
  signal c, d, e, g : logicsig;
  signal t1, t2, t3, t4, t5, t6, t7, t8 : logicsig;
  signal t9, t10, t11, t12, t13, t14 : logicsig;
begin  -- gates

  u1 : cxreceiver port map (
    a => p11,
    y => t1);
  u2 : inv2 port map (
    a  => p6,
    y2 => t2);
  u3 : g2 port map (
    a => p2,
    b => t2,
    y => t3);
  u4 : r2sflop port map (
    s  => t1,
    r  => t3,
    r2 => p12,
    q  => t4,
    qb => c);
  tp1 <= t4;
  u5 : inv port map (
    a => t4,
    y => p13);
  u6 : inv port map (
    a => p4,
    y => t5);
  tp2 <= t5;
  u7 : g3 port map (
    a  => c,
    b  => t5,
    c  => p5,
    y2 => t6);
  u8 : inv port map (
    a => p7,
    y => t7);
  u9 : g2 port map (
    a => t7,
    b => t6,
    y => d);
  u10 : inv2 port map (
    a  => d,
    y2 => p28);
  u11 : g3 port map (
    a => t6,
    b => e,
    c => p9,
    y => t8);
  p17 <= t8;
  u12 : g3 port map (
    a  => g,
    b  => d,
    c  => t8,
    y  => tp6,
    y2 => p18);
  u13 : g5 port map (
    a => t5,
    b => c,
    c => p22,
    d => e,
    e => p10,
    y => g);
  u14 : g2 port map (
    a  => d,
    b  => g,
    y2 => p20);
  u15 : inv2 port map (
    a  => g,
    y2 => p15);
  u16 : inv port map (
    a => p22,
    y => t9);
  u17 : inv port map (
    a => p24,
    y => t10);
  u18 : g2 port map (
    a  => p23,
    b  => t10,
    y  => p27,
    y2 => e);
  u19 : g2 port map (
    a  => t10,
    b  => p21,
    y  => p25,
    y2 => t11);
  u20 : g2 port map (
    a => t11,
    b => p26,
    y => t12);
  p19 <= t12;
  u21 : g3 port map (
    a => p10,
    b => t9,
    c => e,
    y => t13);
  p8 <= t13;
  u22 : g3 port map (
    a  => g,
    b  => t13,
    c  => t12,
    y  => tp5,
    y2 => t14);
  p14 <= t14;
  p16 <= t14;
  p3 <= t14;
  u23 : inv port map (
    a => t14,
    y => p1);
  
end gates;
