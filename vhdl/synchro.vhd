-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell, Al Kossow
--
-- Based on the original design by Seymour Cray and his team
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

begin  -- synchro

  -- purpose: Simulate 6600 synchronizer, connected to DtCyber
  -- type   : sequential
  -- inputs : p2, p3
  -- outputs: p1
  sync: process
    constant idle : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
    variable icv : coaxsigs;
    variable ocv : coaxsigs;
  begin  -- process
    wait on p2(16);
    if p2(16)'event and p2(16) = '0' then
      ocv := p3;
      wait for 10 ns;
      for i in ocv'range loop
        ocv(i) := ocv(i) or p3(i);
      end loop;  -- i
      
      icv := idle;
      dtsynchro (chnum, icv, ocv);
      p1 <= transport icv;
      p1 <= transport idle after 25 ns;
      
    end if;
  end process;

end synchro;
