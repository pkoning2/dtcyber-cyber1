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
-- Synchronizer that connects to DtCyber emulated I/O device
--
-------------------------------------------------------------------------------


library IEEE;
use IEEE.numeric_bit.all;
use work.sigs.all;

entity dtsynchro is
  
  generic (
    chnum : integer);                   -- connected channel number

  port (
    p1 : out coaxsigs;                  -- input cable to channel
    p2 : in coaxsigs;                   -- clock wires on the above
    p3 : in coaxsigs);                  -- output cable from channel

end dtsynchro;

architecture synchro of dtsynchro is
  component ireg 
    port (
      clr : in bit;                       -- clear pulse
      ibus : in coaxbus;                  -- input bus
      obus : out coaxbus);                -- output bus
  end component;
  signal icv, ocv : coaxsigs := idlecoax;
begin  -- synchro
  -- purpose: input latch
  -- type   : combinational
  -- inputs : p3, p2(16)
  -- outputs: ocv
  ilatch : ireg port map (
    ibus => p3,
    clr  => p2(16),
    obus => ocv);
  -- purpose: Simulate 6600 synchronizer, connected to DtCyber
  -- type   : sequential
  -- inputs : p2, ocv
  -- outputs: icv
  sync: process (p2(16))
    variable t_icv, t_ocv : coaxsigs;
  begin  -- process
    if rising_edge (p2(16)) then
      t_icv := idlecoax;
      t_ocv := ocv;
      dtconn (chnum, t_icv, t_ocv);
      icv <= t_icv;
    end if;
  end process;
  inputpulses: for i in icv'range generate
    p1(i) <= icv(i) and p2(16);
  end generate;
end synchro;
