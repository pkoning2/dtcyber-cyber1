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
-- PR module, rev B -- channel data register
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity prslice is
  
  port (
    d, clk : in  logicsig;                   -- data, clock
    idata  : in  coaxsig;                     -- input data coax
    r      : in  logicsig;                   -- reset
    tp     : out logicsig;                   -- test point
    odata  : out coaxsig;                     -- output data coax
    qb     : out logicsig);                  -- output (negated)

end prslice;

architecture gates of prslice is
  component g2
    port (
      a, b : in  logicsig;                   -- inputs
      y, y2    : out logicsig);                  -- output
  end component;
  component cxdriver
    port (
      a : in  logicsig;                        -- source
      y : out coaxsig);                         -- destination
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                    -- source
      y : out logicsig);                 -- destination
  end component;
  component rs2flop
    port (
      s, s2, r  : in  logicsig;                  -- inputs
      q, qb : out logicsig);                 -- outputs
  end component;
  signal ts  : logicsig;
  signal ti : logicsig;         -- internal copy of coax in
begin  -- gates

  u1 : g2 port map (
    a => clk,
    b => d,
    y => ts);
  u2 : cxreceiver port map (
    a => idata,
    y => ti);
  u3 : rs2flop port map (
    s  => ts,
    s2 => ti,
    r  => r,
    q  => tp,
    qb => qb);
  u4 : cxdriver port map (
    a => ts,
    y => odata);
  
end gates;

use work.sigs.all;

entity pr is
  
  port (
    p19 : in  logicsig;               -- clock
    p14 : in  logicsig;               -- clear
    p17 : in  logicsig;               -- bit 0
    p6  : in  coaxsig := '1';                 -- coax data in bit 0
    p5  : out coaxsig;                        -- coax data out bit 0
    p8  : out logicsig;                      -- registered data 0
    p15 : in  logicsig;               -- bit 1
    p4  : in  coaxsig := '1';                 -- coax data in bit 1
    p11 : out coaxsig;                        -- coax data out bit 1
    p7  : out logicsig;                      -- registered data 1
    p16 : in  logicsig;               -- bit 2
    p23 : in  coaxsig := '1';                 -- coax data in bit 2
    p28 : out coaxsig;                        -- coax data out bit 2
    p21 : out logicsig;                      -- registered data 2
    p18 : in  logicsig;               -- bit 3
    p27 : in  coaxsig := '1';                 -- coax data in bit 3
    p24 : out coaxsig;                        -- coax data out bit 3
    p22 : out logicsig;                      -- registered data 3
    p2  : in  logicsig;
    p1  : out coaxsig;
    tp1, tp2, tp5, tp6 : out logicsig);      -- test points

end pr;

architecture gates of pr is
  component prslice
    port (
      d, clk : in  logicsig;                   -- data, clock
      idata  : in  coaxsig;                     -- input data coax
      r      : in  logicsig;                   -- reset
      tp     : out logicsig;                   -- test point
      odata  : out coaxsig;                     -- output data coax
      qb     : out logicsig);                  -- output (negated)
  end component;
  component inv
    port (
      a : in  logicsig;                      -- input
      y : out logicsig);                     -- output
  end component;
  component inv2
    port (
      a  : in  logicsig;                     -- input
      y, y2 : out logicsig);                    -- output
  end component;
  component cxdriver
    port (
      a : in  logicsig;                      -- source
      y : out coaxsig);                       -- destination
  end component;
  signal a, b : logicsig;                    -- buffered clock
begin  -- gates

  u1 : inv port map (
    a => p14,
    y => a);
  u2 : inv2 port map (
    a  => p19,
    y2 => b);
  u3 : prslice port map (
    clk   => b,
    d     => p17,
    r     => a,
    tp    => tp1,
    idata => p6,
    odata => p5,
    qb    => p8);
  u4 : prslice port map (
    clk   => b,
    d     => p15,
    r     => a,
    tp    => tp2,
    idata => p4,
    odata => p11,
    qb    => p7);
  u5 : prslice port map (
    clk   => b,
    d     => p16,
    r     => a,
    tp    => tp5,
    idata => p23,
    odata => p28,
    qb    => p21);
  u6 : prslice port map (
    clk   => b,
    d     => p18,
    r     => a,
    tp    => tp6,
    idata => p27,
    odata => p24,
    qb    => p22);
  u7 : cxdriver port map (
    a => p2,
    y => p1);
end gates;

