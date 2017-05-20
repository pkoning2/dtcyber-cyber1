-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2017 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-- RN module (variant of RG)
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity rnslice is
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      h : in  logicsig;
      y : out coaxsig);

end rnslice;
architecture gates of rnslice is
  component cxdriver4
    port (
      a : in  logicsig;
      a2 : in  logicsig;
      a3 : in  logicsig;
      a4 : in  logicsig;
      y : out coaxsig);

  end component;

  component g2
    port (
      a : in  logicsig;
      b : in  logicsig;
      y : out logicsig;
      y2 : out logicsig);

  end component;

  signal t1 : logicsig;
  signal t2 : logicsig;
  signal t3 : logicsig;
  signal t4 : logicsig;

begin -- gates
  u1 : g2 port map (
    a => a,
    b => e,
    y => t1);


  u2 : g2 port map (
    a => b,
    b => f,
    y => t2);


  u3 : g2 port map (
    a => c,
    b => g,
    y => t3);


  u4 : g2 port map (
    a => d,
    b => h,
    y => t4);


  u5 : cxdriver4 port map (
    a => t1,
    a2 => t2,
    a3 => t3,
    a4 => t4,
    y => y);



end gates;

use work.sigs.all;

entity rn is
    port (
      p2 : in  logicsig;
      p4 : in  logicsig;
      p7 : in  logicsig;
      p8 : in  logicsig;
      p9 : in  logicsig;
      p10 : in  logicsig;
      p11 : in  logicsig;
      p12 : in  logicsig;
      p16 : in  logicsig;
      p17 : in  logicsig;
      p19 : in  logicsig;
      p20 : in  logicsig;
      p22 : in  logicsig;
      p24 : in  logicsig;
      p26 : in  logicsig;
      p28 : in  logicsig;
      tp2 : out logicsig;
      p3_tp1 : out coaxsig;
      p21_tp6 : out coaxsig;
      p23_p25_tp5 : out coaxsig);

end rn;
architecture gates of rn is
  component inv
    port (
      a : in  logicsig;
      y : out logicsig);

  end component;

  component rnslice
    port (
      a : in  logicsig;
      b : in  logicsig;
      c : in  logicsig;
      d : in  logicsig;
      e : in  logicsig;
      f : in  logicsig;
      g : in  logicsig;
      h : in  logicsig;
      y : out coaxsig);

  end component;

  signal a : logicsig;
  signal b : logicsig;
  signal c : logicsig;
  signal d : logicsig;

begin -- gates
  u1 : inv port map (
    a => p10,
    y => tp2);


  u2 : inv port map (
    a => p9,
    y => b);


  u3 : inv port map (
    a => p12,
    y => c);


  u4 : inv port map (
    a => p11,
    y => d);


  u5 : rnslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p8,
    f => p7,
    g => p4,
    h => p2,
    y => p3_tp1);


  u6 : rnslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p22,
    f => p19,
    g => p20,
    h => p17,
    y => p23_p25_tp5);


  u7 : rnslice port map (
    a => a,
    b => b,
    c => c,
    d => d,
    e => p24,
    f => p28,
    g => p16,
    h => p26,
    y => p21_tp6);



end gates;

