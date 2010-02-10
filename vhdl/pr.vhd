-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2010 by Paul Koning
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
      clk : in  logicsig;
      d : in  logicsig;
      idata : in  coaxsig;
      r : in  logicsig;
      tp : out logicsig;
      odata : out coaxsig;
      qb : out logicsig);

end prslice;
architecture gates of prslice is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

  component cxreceiver
    port (
      a : in  coaxsig;
      y : out logicsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rs2flop
    port (
      r : in  logicsig;
      s : in  logicsig;
      s2 : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal ti : logicsig;
  signal ts : logicsig;

begin -- gates
  u1 : g2 port map (
    a => clk,
    b => d,
    y => ts);


  u2 : cxreceiver port map (
    a => idata,
    y => ti);


  u3 : rs2flop port map (
    r => r,
    s => ts,
    s2 => ti,
    q => tp,
    qb => qb);


  u4 : cxdriver port map (
    a => ts,
    y => odata);



end gates;

use work.sigs.all;

entity pr is
    port (
      p2 : in  logicsig;
      p4 : in  coaxsig := '1';
      p6 : in  coaxsig := '1';
      p14 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p18 : in  logicsig;
      p19 : in  logicsig;
      p23 : in  coaxsig := '1';
      p27 : in  coaxsig := '1';
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p1 : out coaxsig;
      p5 : out coaxsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p11 : out coaxsig;
      p21 : out logicsig;
      p22 : out logicsig;
      p24 : out coaxsig;
      p28 : out coaxsig);

end pr;
architecture gates of pr is
  component cxdriver
    port (
      a : in  logicsig;
      y : out coaxsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component prslice
    port (
      clk : in  logicsig;
      d : in  logicsig;
      idata : in  coaxsig;
      r : in  logicsig;
      tp : out logicsig;
      odata : out coaxsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;

begin -- gates
  u1 : inv port map (
    a => p14,
    y => a);


  u2 : inv2 port map (
    a => p19,
    y2 => b);


  u3 : prslice port map (
    clk => b,
    d => p17,
    idata => p6,
    r => a,
    odata => p5,
    qb => p8,
    tp => tp1);


  u4 : prslice port map (
    clk => b,
    d => p15,
    idata => p4,
    r => a,
    odata => p11,
    qb => p7,
    tp => tp2);


  u5 : prslice port map (
    clk => b,
    d => p16,
    idata => p23,
    r => a,
    odata => p28,
    qb => p21,
    tp => tp5);


  u6 : prslice port map (
    clk => b,
    d => p18,
    idata => p27,
    r => a,
    odata => p24,
    qb => p22,
    tp => tp6);


  u7 : cxdriver port map (
    a => p2,
    y => p1);



end gates;

