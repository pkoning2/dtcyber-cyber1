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
-- Netlist transcribed from 60042700 Printed Circuit Manual, V-Z_1,
-- page 139 (LS 17830500).  Ten set/reset flip-flops; each takes a
-- data input gated by letter A or B (both = NOT P11) as set, and
-- letter C or D (both = P11) as reset.
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
      p28 : out logicsig;
      tp1 : out logicsig;
      tp2 : out logicsig;
      tp3 : out logicsig;
      tp4 : out logicsig;
      tp5 : out logicsig;
      tp6 : out logicsig);
end we;

architecture gates of we is
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
      s : in  logicsig;
      r : in  logicsig;
      q : out logicsig;
      qb : out logicsig);
  end component;

  signal la : logicsig;    -- A = B = NOT P11
  signal lc : logicsig;    -- C = D = P11
  signal s1 : logicsig;    -- NAND(P1,A)
  signal s3 : logicsig;
  signal s5 : logicsig;
  signal s7 : logicsig;
  signal s9 : logicsig;
  signal s19 : logicsig;
  signal s21 : logicsig;
  signal s23 : logicsig;
  signal s25 : logicsig;
  signal s27 : logicsig;
  signal q2 : logicsig;
  signal q4 : logicsig;
  signal q6 : logicsig;
  signal q8 : logicsig;
  signal q10 : logicsig;
  signal q20 : logicsig;
  signal q22 : logicsig;
  signal q24 : logicsig;
  signal q26 : logicsig;
  signal q28 : logicsig;

begin -- gates
  -- letters
  u1 : inv2 port map (a => p11, y => la, y2 => lc);

  -- P2 (no triangle)
  u2 : g2 port map (a => p1, b => la, y => s1);
  u3 : rsflop port map (s => s1, r => lc, q => q2);

  -- P4, triangle 1
  u4 : g2 port map (a => p3, b => la, y => s3);
  u5 : rsflop port map (s => s3, r => lc, q => q4);
  tp1 <= q4;

  -- P6, triangle 3
  u6 : g2 port map (a => p5, b => la, y => s5);
  u7 : rsflop port map (s => s5, r => lc, q => q6);
  tp3 <= q6;

  -- P8 (no triangle)
  u8 : g2 port map (a => p7, b => la, y => s7);
  u9 : rsflop port map (s => s7, r => lc, q => q8);

  -- P10 (no triangle)
  u10 : g2 port map (a => p9, b => la, y => s9);
  u11 : rsflop port map (s => s9, r => lc, q => q10);

  -- P20, triangle 6
  u12 : g2 port map (a => p19, b => la, y => s19);
  u13 : rsflop port map (s => s19, r => lc, q => q20);
  tp6 <= q20;

  -- P22, triangle 4
  u14 : g2 port map (a => p21, b => la, y => s21);
  u15 : rsflop port map (s => s21, r => lc, q => q22);
  tp4 <= q22;

  -- P24, triangle 5
  u16 : g2 port map (a => p23, b => la, y => s23);
  u17 : rsflop port map (s => s23, r => lc, q => q24);
  tp5 <= q24;

  -- P26 (no triangle)
  u18 : g2 port map (a => p25, b => la, y => s25);
  u19 : rsflop port map (s => s25, r => lc, q => q26);

  -- P28 (no triangle)
  u20 : g2 port map (a => p27, b => la, y => s27);
  u21 : rsflop port map (s => s27, r => lc, q => q28);

  -- triangle 2 is letter C (= P11)
  tp2 <= lc;

  p2 <= q2;
  p4 <= q4;
  p6 <= q6;
  p8 <= q8;
  p10 <= q10;
  p20 <= q20;
  p22 <= q22;
  p24 <= q24;
  p26 <= q26;
  p28 <= q28;

end gates;
