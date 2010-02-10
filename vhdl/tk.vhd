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
-- TK module, rev C -- 3 to 8 decode
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tk is
    port (
      p9 : in  logicsig;
      p18 : in  logicsig;
      p26 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p3 : out logicsig;
      p6 : out logicsig;
      p7 : out logicsig;
      p8 : out logicsig;
      p11 : out logicsig;
      p22 : out logicsig;
      p25 : out logicsig;
      p27 : out logicsig);

end tk;
architecture gates of tk is
  component g3
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component inv2
    port (
      a : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;
  signal e : logicsig;
  signal f : logicsig;

begin -- gates
  u1 : inv2 port map (
    a => p26,
    y => a,
    y2 => b);


  u2 : inv2 port map (
    a => p18,
    y => c,
    y2 => d);


  u3 : inv2 port map (
    a => p9,
    y => e,
    y2 => f);


  u4 : g3 port map (
    a => a,
    b => c,
    c => e,
    y => tp1,
    y2 => p11);


  u5 : g3 port map (
    a => a,
    b => c,
    c => f,
    y2 => p8);


  u6 : g3 port map (
    a => a,
    b => d,
    c => e,
    y => tp2,
    y2 => p22);


  u7 : g3 port map (
    a => a,
    b => d,
    c => f,
    y2 => p27);


  u8 : g3 port map (
    a => b,
    b => c,
    c => f,
    y2 => p3);


  u9 : g3 port map (
    a => b,
    b => d,
    c => e,
    y => tp5,
    y2 => p7);


  u10 : g3 port map (
    a => b,
    b => c,
    c => e,
    y => tp6,
    y2 => p25);


  u11 : g3 port map (
    a => b,
    b => d,
    c => f,
    y2 => p6);



end gates;

