-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

library IEEE;
use IEEE.std_logic_1164.all;

use work.sigs.all;

entity synchro is
  
  generic (
    chnum : integer);                   -- connected channel number

  port (
    p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12 : out coaxsig;  -- data
    p13, p14, p15, p16 : out coaxsig;           -- active, inactive, full, empty
    p17, p18: in coaxsig := '0';                -- 10 and 1 MHz clocks
    p101, p102, p103, p104, p105, p106, p107 : in coaxsig;
    p108, p109, p110, p111, p112 : in coaxsig;  -- data
    p113, p114, p115, p116, p117, p118 : in coaxsig);  -- control outputs

end synchro;

architecture synchro of synchro is

begin  -- synchro

  -- purpose: Simulate 6600 synchronizer, connected to DtCyber
  -- type   : sequential
  -- inputs : p17, p18, p101 - p118
  -- outputs: p1 - p16
  process (p17)
    constant idle : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
    variable icv : coaxsigs;
    variable ocv : coaxsigs;
  begin  -- process
    if p17'event and p17 = '0' then  -- trailing clock edge
      ocv := ( p101, p102, p103, p104, p105, p106, p107, p108, p109, p110,
               p111, p112, p113, p114, p115, p116, p117, p118, '0' );
      icv := idle;
      dtsynchro (chnum, icv, ocv);
      p1 <= transport icv (1); p1 <= transport '0' after 25 ns;
      p2 <= transport icv (2); p2 <= transport '0' after 25 ns;
      p3 <= transport icv (3); p3 <= transport '0' after 25 ns;
      p4 <= transport icv (4); p4 <= transport '0' after 25 ns;
      p5 <= transport icv (5); p5 <= transport '0' after 25 ns;
      p6 <= transport icv (6); p6 <= transport '0' after 25 ns;
      p7 <= transport icv (7); p7 <= transport '0' after 25 ns;
      p8 <= transport icv (8); p8 <= transport '0' after 25 ns;
      p9 <= transport icv (9); p9 <= transport '0' after 25 ns;
      p10 <= transport icv (10); p10 <= transport '0' after 25 ns;
      p11 <= transport icv (11); p11 <= transport '0' after 25 ns;
      p12 <= transport icv (12); p12 <= transport '0' after 25 ns;
      p13 <= transport icv (13); p13 <= transport '0' after 25 ns;
      p14 <= transport icv (14); p14 <= transport '0' after 25 ns;
      p15 <= transport icv (15); p15 <= transport '0' after 25 ns;
      p16 <= transport icv (16); p16 <= transport '0' after 25 ns;
    end if;
  end process;

end synchro;
