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
-- TG module, rev H -- 4 input mux
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity tgslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      q : out logicsig;
      qb : out logicsig);

end tgslice;
architecture gates of tgslice is
  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  component g4
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal ti : logicsig;
  signal ti1 : logicsig;
  signal ti2 : logicsig;
  signal ti3 : logicsig;
  signal ti4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => i1,
    b => a,
    y => ti1);


  u2 : g2 port map (
    a => i2,
    b => b,
    y => ti2);


  u3 : g2 port map (
    a => i3,
    b => c,
    y => ti3);


  u4 : g2 port map (
    a => i4,
    b => d,
    y => ti4);


  u5 : g4 port map (
    a => ti1,
    b => ti2,
    c => ti3,
    d => ti4,
    y => ti,
    y2 => qb);

  q <= ti;
  tp <= ti;


end gates;

use work.sigs.all;

entity tg is
    port (
      p1 : in  logicsig;
      p2 : in  logicsig;
      p3 : in  logicsig;
      p4 : in  logicsig;
      p5 : in  logicsig;
      p6 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p15 : in  logicsig;
      p16 : in  logicsig;
      p21 : in  logicsig;
      p22 : in  logicsig;
      p23 : in  logicsig;
      p24 : in  logicsig;
      p25 : in  logicsig;
      p26 : in  logicsig;
      p27 : in  logicsig;
      p28 : in  logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig;
      p9 : out logicsig;
      p10 : out logicsig;
      p13 : out logicsig;
      p14 : out logicsig;
      p17 : out logicsig;
      p18 : out logicsig;
      p19 : out logicsig;
      p20 : out logicsig);

end tg;
architecture gates of tg is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component tgslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      i1 : in  logicsig;
      i2 : in  logicsig;
      i3 : in  logicsig;
      i4 : in  logicsig;
      tp : out logicsig;
      q : out logicsig;
      qb : out logicsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv port map (
    a => p12,
    y => a);


  u2 : inv port map (
    a => p16,
    y => b);


  u3 : inv port map (
    a => p11,
    y => c);


  u4 : inv port map (
    a => p15,
    y => d);


  u5 : tgslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p2,
    i2 => p4,
    i3 => p6,
    i4 => p8,
    q => p13,
    qb => p14,
    tp => tp1);


  u6 : tgslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p1,
    i2 => p3,
    i3 => p5,
    i4 => p7,
    q => p10,
    qb => p9,
    tp => tp2);


  u7 : tgslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p28,
    i2 => p26,
    i3 => p24,
    i4 => p22,
    q => p17,
    qb => p18,
    tp => tp5);


  u8 : tgslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    i1 => p27,
    i2 => p25,
    i3 => p23,
    i4 => p21,
    q => p20,
    qb => p19,
    tp => tp6);



end gates;

