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

-------------------------------------------------------------------------------
-- Top level of CDC6600 model

entity cdc6600 is
  
  port (
    testdata : in testvector);          -- test data from testbed

end cdc6600;

architecture gates of cdc6600 is
  component clock
    port (
      I, II, III, IV : out std_logic);    -- The four clock phases
  end component;
begin  -- gates

  

end gates;
