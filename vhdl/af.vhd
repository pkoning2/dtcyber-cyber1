-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
--
-- AF module -- 6612 D-A circuit
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.sigs.all;

entity afslice is
  
  port (
    a, b, c, d, e, f : in  std_logic;
    tp               : out analog;      -- test point
    q                : out analog);

end afslice;

architecture beh of afslice is
  function dtoa (
    a, b, c, d, e, f : std_logic)       -- inputs
    return integer is
    variable acc : integer;
  begin  -- dtoa
    acc := 0;
    if a = '1' then
      acc := acc + 1;
    end if;
    if b = '1' then
      acc := acc + 1;
    end if;
    if c = '1' then
      acc := acc + 1;
    end if;
    if d = '1' then
      acc := acc + 1;
    end if;
    if e = '1' then
      acc := acc + 1;
    end if;
    if f = '1' then
      acc := acc + 1;
    end if;
    return acc;
  end dtoa;
begin  -- beh
  q <= TO_UNSIGNED (dtoa (a, b, c, d, e, f), 3);
end beh;


library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use work.sigs.all;

entity af is
  
  port (
    p2, p4, p6, p8, p10, p12     : in  std_logic;
    p1, p3, p5, p7, p9, p11      : in  std_logic;
    p18, p20, p22, p24, p26, p28 : in  std_logic;
    p17, p19, p21, p23, p25, p27 : in  std_logic;
    tp1, tp2, tp5, tp6           : out analog;  -- test points
    p14, p13, p16, p15           : out analog);

end af;

architecture gates of af is
  component afslice
    port (
      a, b, c, d, e, f : in  std_logic;
      tp               : out analog;      -- test point
      q                : out analog);
  end component;
begin  -- gates

  u1 : afslice port map (
    a  => p2,
    b  => p4,
    c  => p6,
    d  => p8,
    e  => p10,
    f  => p12,
    tp => tp1,
    q  => p14);
  u2 : afslice port map (
    a  => p1,
    b  => p3,
    c  => p5,
    d  => p7,
    e  => p9,
    f  => p11,
    tp => tp2,
    q  => p13);
  u3 : afslice port map (
    a  => p18,
    b  => p20,
    c  => p22,
    d  => p24,
    e  => p26,
    f  => p28,
    tp => tp5,
    q  => p16);
  u4 : afslice port map (
    a  => p17,
    b  => p19,
    c  => p21,
    d  => p23,
    e  => p25,
    f  => p27,
    tp => tp6,
    q  => p15);

end gates;
