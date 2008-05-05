-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-- TK module, rev C -- 3 to 8 decode
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use work.sigs.all;

entity tk is
  
  port (
    p26, p18, p9       : in  std_logic;
    tp1, tp2, tp5, tp6 : out std_logic;  -- test points
    p11, p8, p22, p27  : out std_logic;
    p3, p7, p25, p6    : out std_logic);

end tk;

architecture gates of tk is
  component inv2
    port (
      a  : in  std_logic;                     -- input
      y, y2 : out std_logic);                    -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      y, y2   : out std_logic);                  -- output
  end component;
  signal a, b, c, d, e, f : std_logic;
begin  -- gates

  u1 : inv2 port map (
    a  => p26,
    y  => a,
    y2 => b);
  u2 : inv2 port map (
    a  => p18,
    y  => c,
    y2 => d);
  u3 : inv2 port map (
    a  => p9,
    y  => e,
    y2 => f);
  u4 : g3 port map (
    a  => a,
    b  => c,
    c  => e,
    y  => tp1,
    y2 => p11);
  u5 : g3 port map (
    a  => a,
    b  => c,
    c  => f,
    y2 => p8);
  u6 : g3 port map (
    a  => a,
    b  => d,
    c  => e,
    y  => tp2,
    y2 => p22);
  u7 : g3 port map (
    a  => a,
    b  => d,
    c  => f,
    y2 => p27);
  u8 : g3 port map (
    a  => b,
    b  => c,
    c  => f,
    y2 => p3);
  u9 : g3 port map (
    a  => b,
    b  => d,
    c  => e,
    y  => tp5,
    y2 => p7);
  u10 : g3 port map (
    a  => b,
    b  => c,
    c  => e,
    y  => tp6,
    y2 => p25);
  u11 : g3 port map (
    a  => b,
    b  => d,
    c  => f,
    y2 => p6);

end gates;
