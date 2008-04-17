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
    ic : inout coaxsigs;                -- input cable
    oc : inout coaxsigs);               -- output cable

end synchro;

architecture synchro of synchro is

begin  -- synchro

  -- purpose: Simulate 6600 synchronizer, connected to DtCyber
  -- type   : sequential
  -- inputs : ic(17), oc
  -- outputs: ic(1 to 16)
  process (ic (17))
    constant idle : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
    variable icv : coaxsigs;
    variable ocv : coaxsigs;
  begin  -- process
    ic (17) <= 'Z';                     -- because these come from 6600
    ic (18) <= 'Z';
    if ic (17)'event and ic (17) = '0' then  -- trailing clock edge
      ocv := oc;
      icv := idle;
      dtsynchro (chnum, icv, ocv);
      for i in 1 to 16 loop
        ic(i) <= transport icv(i);
        ic(i) <= transport '0' after 25 ns;
      end loop;  -- i
    end if;
  end process;

end synchro;
