-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2008-2009 by Paul Koning
--
-- Derived from the original 6600 module design
-- by Seymour Cray and his team at Control Data,
-- as documented in CDC 6600 "Chassis Tabs" manuals,
-- which are in the public domain.  Scans supplied
-- from the Computer History Museum collection
-- by Dave Redell and Al Kossow.
--
-------------------------------------------------------------------------------


use work.sigs.all;

entity synchro is
  
  generic (
    chnum : integer);                   -- connected channel number

  port (
    p1 : out coaxsigs;                  -- input cable to channel
    p2 : in coaxsigs;                   -- clock wires on the above
    p3 : in coaxsigs);                  -- output cable from channel

end synchro;

architecture synchro of synchro is
  signal ocv : coaxsigs := idlecoax;
begin  -- synchro
  -- purpose: input latch
  -- type   : combinational
  -- inputs : p3, p2(16)
  -- outputs: ocv
  ilatch: process (p3, p2(16))
  begin  -- process ilatch
    if p2(16) = '1' then
      ocv <= idlecoax;
    end if;
    for i in p3'range loop
      if p3(i) = '1' then
        ocv(i) <= '1';
      end if;
    end loop;
  end process ilatch;
  -- purpose: Simulate 6600 synchronizer, connected to DtCyber
  -- type   : sequential
  -- inputs : p2, ocv
  -- outputs: p1
  sync: process (p2(16))
    variable icv, t : coaxsigs;
  begin  -- process
    if p2(16)'event and p2(16) = '1' then
      icv := idlecoax;
      t := ocv;
      dtsynchro (chnum, icv, t);
      p1 <= icv, idlecoax after 25 ns;
    end if;
  end process;

end synchro;
