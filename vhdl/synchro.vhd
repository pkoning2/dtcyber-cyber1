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
    clk10 : in std_logic;               -- 10 MHz clock
    ic : inout coaxsigs;                -- input cable
    oc : inout coaxsigs);               -- output cable

end synchro;

architecture synchro of synchro is

begin  -- synchro

  -- purpose: Simulate 6600 synchronizer, connected to DtCyber
  -- type   : sequential
  -- inputs : clk10, ic, oc
  -- outputs: ic, oc
  process (clk10)
    constant idle : coaxsigs := ('1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1','1');
    variable icv : coaxsigs;
    variable ocv : coaxsigs;
  begin  -- process
    -- Note that logic states are inverted on simulated coax
    if clk10'event and clk10 = '1' then  -- trailing clock edge
      ocv := oc;
      icv := idle;
      dtsynchro (chnum, icv, ocv);
      ic <= transport icv;
      ic <= transport idle after 25 ns;
    end if;
  end process;

end synchro;
