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
-- PZ module, rev E -- memory inhibit driver
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity pzslice is
    port (
      clk : in  logicsig;
      d : in  logicsig;
      r : in  logicsig;
      tp : out logicsig;
      q : out logicsig);

end pzslice;
architecture gates of pzslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component rsflop
    port (
      r : in  logicsig;
      s : in  logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => d,
    b => clk,
    y => t1);


  u2 : rsflop port map (
    r => r,
    s => t1,
    q => t2);

  tp <= t2;

  u3 : inv2 port map (
    a => t2,
    y2 => q);



end gates;

use work.sigs.all;

entity pz is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p6 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p22 : in  logicsig;
      p27 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p5 : out logicsig;
      p7 : out logicsig;
      p21 : out logicsig;
      p23 : out logicsig;
      p25 : out logicsig);

end pz;
architecture gates of pz is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component pzslice
    port (
      clk : in  logicsig;
      d : in  logicsig;
      r : in  logicsig;
      tp : out logicsig;
      q : out logicsig);

  end component;

  signal a : logicsig;
  signal c : logicsig;
  signal t1 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => p17,
    b => p9,
    y2 => a);


  u2 : g2 port map (
    a => p16,
    b => p10,
    y2 => t1);


  u3 : pzslice port map (
    clk => a,
    d => p4,
    r => c,
    q => p5,
    tp => tp1);


  u4 : pzslice port map (
    clk => a,
    d => p2,
    r => c,
    q => p3,
    tp => tp2);


  u5 : pzslice port map (
    clk => a,
    d => p6,
    r => c,
    q => p7,
    tp => tp3);


  u6 : pzslice port map (
    clk => a,
    d => p19,
    r => c,
    q => p21,
    tp => tp4);


  u7 : pzslice port map (
    clk => a,
    d => p27,
    r => c,
    q => p25,
    tp => tp5);


  u8 : pzslice port map (
    clk => a,
    d => p22,
    r => c,
    q => p23,
    tp => tp6);


  u9 : inv port map (
    a => t1,
    y => c);



end gates;

