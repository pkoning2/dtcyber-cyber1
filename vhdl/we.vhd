-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2018 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- WE module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity we is
    port (
      p1 : in  logicsig;
      p3 : in  logicsig;
      p5 : in  logicsig;
      p7 : in  logicsig;
      p9 : in  logicsig;
      p11 : in  logicsig;
      p19 : in  logicsig;
      p21 : in  logicsig;
      p23 : in  logicsig;
      p25 : in  logicsig;
      p27 : in  logicsig;
      p2 : out logicsig;
      p4 : out logicsig;
      p6 : out logicsig;
      p8 : out logicsig;
      p10 : out logicsig;
      p20 : out logicsig;
      p22 : out logicsig;
      p24 : out logicsig;
      p26 : out logicsig;
      p28 : out logicsig);

end we;
architecture gates of we is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component latch
    port (
      clk : in  logicsig;
      d : in  logicsig;
      q : out logicsig;
      qb : out logicsig;
      qs : out logicsig);

  end component;

  signal t1 : logicsig;

begin -- gates
  u1 : inv port map (
    a => p11,
    y => t1);


  u2 : latch port map (
    clk => t1,
    d => p1,
    q => p2);


  u3 : latch port map (
    clk => t1,
    d => p3,
    q => p4);


  u4 : latch port map (
    clk => t1,
    d => p5,
    q => p6);


  u5 : latch port map (
    clk => t1,
    d => p7,
    q => p8);


  u6 : latch port map (
    clk => t1,
    d => p9,
    q => p10);


  u7 : latch port map (
    clk => t1,
    d => p19,
    q => p20);


  u8 : latch port map (
    clk => t1,
    d => p21,
    q => p22);


  u9 : latch port map (
    clk => t1,
    d => p23,
    q => p24);


  u10 : latch port map (
    clk => t1,
    d => p25,
    q => p26);


  u11 : latch port map (
    clk => t1,
    d => p27,
    q => p28);



end gates;

