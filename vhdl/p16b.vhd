
-- eM24
-- em24a	03jun00cht, expand cpu16 to 24 bits
-- em24e1	11jun00cht, add sks, skr, lld, lst
-- em24e2	11jun00cht, use named signals in muxes
-- em24e3	13jun00cht, bidirectional data bus
-- em24e4	15jun00cht, rearrange opcode, add test_in, test_out
-- em24f1	10jul00cht, new instruction set, failed
-- em24f2	11jul00cht, start from e4, proceed slowly
-- em24f3	14jul00cht, consolidate incrementors p,x,y. no good.
-- em24f4	14jul00cht, eliminate p-mux, no good
-- em24f5	14jul00cht, change shr for no-cost uart, decy now is next.
--		16jul00cht, change depth to 15.
--		17jul00cht, change depth back to 7, delete test_in/out.
-- em24f6	06aug00cht, xy/sr_depth, change n to s.
-- em25a1	06aug00cht, use core generator for stacks.
-- em25a2	07aug00cht, use ramb4_s16_s16 for stacks.
-- em25a3	09aug00cht, add code to memory
-- em25a4	10aug00cht, eni to replace bbt
-- em25a5	14aug00cht, code in memory
-- em25a6	14aug00cht, change to ldrp and strp, fix slot
--		useattribute for code initialization
--		16aug00cht, test Diagnose
-- em25a7	18aug00cht, eliminate r,x,y registers
-- cpu24a1	01sep00cht, adapted from em25a7 and cpu16
-- cpu24b	07nov00cht, real tests
--			change x back to a.
--			put stacks back to logic.
-- p24c		02dec00cht, 6 bit instructions, mul, div, interrupts
--		05dec00cht, move to vcx1000e
-- p24d		06dec00cht, move to vcx300
-- p24c		interrupt removed, iload and uload fixed.  working.
-- p24e		13dec00cht, test core-gen ram and mif file.
-- p16b		11jan01cht, make p16
-- p16c		18jan01cht, expand stacks to 32 levels.
--              17dec09gpk, use numeric_std package, enums for mux selects,
 --              other cleanup

library ieee;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;


entity cpu16 is 
  generic(width: integer := 16;
          stacksize: integer := 32);
  port(
    holdosc0: out std_logic;
    clk, arst, uart_in: in std_logic;
    icode: out unsigned(4 downto 0);
    interrupt: in unsigned(4 downto 0);
    uart_out, auload: out std_logic;
    data: out unsigned(width - 1 downto 0);
    address: out unsigned(11 downto 0));
end entity cpu16;

architecture archcpu16 of cpu16 is

  component p16ram
    port (
      addr: IN unsigned(11 downto 0);
      clk: IN std_logic;
      di: IN unsigned(width - 1 downto 0);
      we: IN std_logic;
      en: IN std_logic;
      rst: IN std_logic;
      do: OUT unsigned(width - 1 downto 0));
  end component;

  -- mux to t register, selected by t_sel
  type t_sel_t is (not_t, s_xor_t, s_and_t, sum_t, shr_sum_t,
                   shr_t, shr_t_t, shl_sum_a_t, shl_t_a_t, shl_t,
                   s_t, a_t, r_t, data_t );
  -- mux to a register, selected by a_sel
  type a_sel_t is (t_a, a1_a, shr_sum_a, shr_t_a, shl_sum_a);
  -- mux to r register, selected by r_sel
  type r_sel_t is (t_r, p_r);
  -- mux to p register, selected by p_sel
  type p_sel_t is (p1_p, pi_p, r_p, int_p);
  -- mux to memory bus, selected by addr_sel
  type addr_sel_t is (p_addr, a_addr);

  signal slot: integer range 0 to 3;
  type stack is array(stacksize - 1 downto 0) of unsigned(width downto 0);
  signal as_stack, ar_stack: stack;
  signal sp, sp1, rp, rp1: integer range 0 to stacksize - 1;
  signal t, s, i, p, sum: unsigned(width downto 0);
  signal a, r: unsigned(width downto 0);
  signal t_in, s_in, r_in, a_in, p_in: unsigned(width downto 0);
  signal skip_sel: unsigned(5 downto 0);
  signal code: unsigned(4 downto 0);
  signal t_sel: t_sel_t;
  signal p_sel: p_sel_t;
  signal a_sel: a_sel_t;
  signal r_sel: r_sel_t;
  signal spush, spopp, rpush, rpopp,
    tload, aload, pload, iload, reset, z: std_logic;
  signal addr_sel : addr_sel_t;
  signal write, uload: std_logic;
  signal clk0, clr: std_logic;
  signal addr: unsigned(11 downto 0);
  signal data_out, data_in: unsigned(width - 1 downto 0);
  
-- machine instructions selected by code
  constant jmp : unsigned(4 downto 0) :="00000";
  constant ret : unsigned(4 downto 0) :="00001";
  constant bz  : unsigned(4 downto 0) :="00010";
  constant bnc : unsigned(4 downto 0) :="00011";
  constant call: unsigned(4 downto 0) :="00100";
  constant ldp : unsigned(4 downto 0) :="01001";
  constant lit : unsigned(4 downto 0) :="01010";
  constant ld  : unsigned(4 downto 0) :="01011";
  constant stp : unsigned(4 downto 0) :="01101";
  constant st  : unsigned(4 downto 0) :="01111";
  constant com : unsigned(4 downto 0) :="10000";
  constant shl : unsigned(4 downto 0) :="10001";
  constant shr : unsigned(4 downto 0) :="10010";
  constant addc: unsigned(4 downto 0) :="10011";
  constant xorr: unsigned(4 downto 0) :="10100";
  constant andd: unsigned(4 downto 0) :="10101";
  constant addd: unsigned(4 downto 0) :="10111";
  constant pop : unsigned(4 downto 0) :="11000";
  constant lda : unsigned(4 downto 0) :="11001";
  constant dup : unsigned(4 downto 0) :="11010";
  constant push: unsigned(4 downto 0) :="11100";
  constant sta : unsigned(4 downto 0) :="11101";
  constant nop : unsigned(4 downto 0) :="11110";
  constant drop: unsigned(4 downto 0) :="11111";

begin
-- concurent assignments, mostly with muxes

  auload <= uload;
  address <= addr;
  clk0 <= not clk;
  clr <= not arst;
  holdosc0 <= arst;	
  r <= ar_stack(rp);
  s <= as_stack(sp);
  data <= data_out;
  data_in <= t(width - 1 downto 0);

------------- Begin Cut here for INSTANTIATION Template ----- INST_TAG
  memory : p16ram
    port map (
      addr => addr,
      clk => clk0,
      di => data_in,
      we => write,
      en => arst,
      rst => clr,
      do => data_out);
-- INST_TAG_END ------ End INSTANTIATION Template ------------

  icode <= code;

  sum <= (('0' & t(width - 1 downto 0)) + ('0' & s(width - 1 downto 0)));

  with t_sel select
    t_in <= (not t) when not_t,
    (t xor s) when s_xor_t,
    (t and s) when s_and_t,
    sum when sum_t,
    (t(width - 1 downto 0) & '0') when shl_t,
    (t(width - 1 downto 0) & a(width - 1)) when shl_t_a_t,
    (sum(width - 1 downto 0) & a(width - 1)) when shl_sum_a_t,
    ('0' & sum(width downto 1)) when shr_sum_t,
    (uart_in & t(width - 1) & t(width - 1 downto 1)) when shr_t,
    ("00" & t(width - 1 downto 1)) when shr_t_t,
    s when s_t,
    a when a_t,
    r when r_t,
    '0' & data_out(width - 1 downto 0) when data_t;

  with slot select
    code <= i(width - 1 downto 11) when 1,
    i(10 downto 6) when 2,
    i(5 downto 1) when 3,
    nop when others;

  with a_sel select
    a_in <= a + 1 when a1_a,
    ('0' & t(0) & a(width - 1 downto 1)) when shr_t_a,
    ('0' & sum(0) & a(width - 1 downto 1)) when shr_sum_a,
    ('0' & a(width - 2 downto 0) & sum(width)) when shl_sum_a,
    t when t_a;

  with r_sel select
    r_in <= p when p_r,
    t when t_r; 

  with p_sel select
    p_in <= (p(width downto 11) & i(10 downto 0)) when pi_p,
    r when r_p,
    ("000000000000" & interrupt(4 downto 0)) when int_p,
    p + 1 when p1_p;

  with addr_sel select
    addr <= a(11 downto 0) when a_addr,
    p(11 downto 0) when p_addr;


  z <= not(t(15) or t(14) or t(13) or t(12)
           or t(11) or t(10) or t(9) or t(8)
           or t(7) or t(6) or t(5) or t(4)
           or t(3) or t(2) or t(1) or t(0));

-- sequential assignments, with slot and code		
  decode: process(code, z, a, t, slot, sum, p, i, interrupt,
                  sp, sp1, rp, rp1)
  begin
    t_sel <= t_sel_t'left; 
    a_sel <= a_sel_t'left;
    p_sel <= p_sel_t'left;
    r_sel <= r_sel_t'left;
    spush <= '0'; 
    spopp <= '0';
    rpush <= '0'; 
    rpopp <= '0'; 
    tload <= '0'; 
    aload <= '0';
    pload <= '0'; 
    addr_sel <= addr_sel_t'left; 
    write <= '0'; 
    iload <= '0';
    reset <= '0';
    uload <= '0';

    if slot=0 then
      if interrupt/="00000" then
        pload <= '1';
        p_sel <= int_p;--process interrupts
        rpush <= '1'; 
        r_sel <= p_r;
        reset <= '1';
      else	
        iload <= '1';
        p_sel <= p1_p;--fetch next word
        pload <= '1';
      end if;
    else
      case code is
        when jmp =>
          pload <= '1';
          p_sel <= pi_p;
          reset <= '1';
        when ret =>
          pload <= '1'; 
          p_sel <= r_p;
          rpopp <= '1';
          reset <= '1';
        when bz => 
          if Z='1' then
            pload <= '1';
            p_sel <= pi_p;
          end if;
          reset <= '1';
        when bnc => 
          if t(width)='0' then
            pload <= '1';
            p_sel <= pi_p;
          end if;
          reset <= '1';
        when call => 
          pload <= '1';
          p_sel <= pi_p;--process call
          rpush <= '1'; 
          r_sel <= p_r;
          reset <= '1';
        when ldp =>
          addr_sel <= a_addr; 
          a_sel <= a1_a;
          aload <= '1'; 
          tload <= '1';
          t_sel <= data_t; 
          spush <= '1'; 
        when lit =>
          pload <= '1'; 
          p_sel <= p1_p;
          tload <= '1';
          t_sel <= data_t; 
          spush <= '1'; 
        when ld =>
          addr_sel <= a_addr; 
          tload <= '1';
          t_sel <= data_t; 
          spush <= '1'; 
        when stp =>
          addr_sel <= a_addr; 
          aload <= '1'; 
          a_sel <= a1_a;
          tload <= '1';
          t_sel <= s_t; 
          spopp <= '1'; 
          write <= '1'; 
        when st =>
          addr_sel <= a_addr; 
          tload <= '1';
          t_sel <= s_t; 
          spopp <= '1'; 
          write <= '1'; 
        when com => 
          tload <= '1';
          t_sel <= not_t; 
        when shl => 
          tload <= '1';
          t_sel <= shl_t; 
        when shr => 
          tload <= '1';
          t_sel <= shr_t; 
          uload <= '1';
        when addc => 
          if t(0)='1' then
            t_sel <= sum_t;
            tload <= '1';
          end if;
        when xorr => 
          tload <= '1';
          t_sel <= s_xor_t; 
          spopp <= '1';
        when andd => 
          tload <= '1';
          t_sel <= s_and_t; 
          spopp <= '1';
        when addd => 
          tload <= '1';
          t_sel <= sum_t; 
          spopp <= '1';
        when pop => 
          tload <= '1';
          t_sel <= r_t; 
          spush <= '1';
          rpopp <= '1';
        when lda => 
          tload <= '1';
          t_sel <= a_t; 
          spush <= '1';
        when dup => 
          spush <= '1';
        when push => 
          tload <= '1';
          t_sel <= s_t; 
          rpush <= '1';
          r_sel <= t_r;
          spopp <= '1';
        when sta => 
          tload <= '1';
          t_sel <= s_t; 
          a_sel <= t_a;
          aload <= '1'; 
          spopp <= '1';
        when nop =>
          reset <= '1';
        when drop => 
          tload <= '1';
          t_sel <= s_t; 
          spopp <= '1';
        when others =>
          reset <= '1';
      end case;
    end if;
  end process decode;

-- finite state machine, processor control unit	
  sync: process(clk, clr)
  begin
    if clr='1' then -- master reset
      slot <= 0;
      sp  <= 0;
      sp1 <= 1;
      rp  <= 0;
      rp1 <= 1;
      t <= (others => '0');
      a <= (others => '0');
      p <= (others => '0');
      i <= (others => '0');
      for ii in as_stack'range loop
        as_stack(ii) <= (others => '0');
        ar_stack(ii) <= (others => '0');
      end loop;
    elsif (clk'event and clk='1') then
      if reset='1' or slot=3 then
        slot <= 0;
      else
        slot <= slot + 1;
      end if;
      if iload='1' then
        i <= '0' & data_out(width - 1 downto 0);
      end if;
      if pload='1' then
        p <= p_in;
      end if;
      if tload='1' then
        t <= t_in;
      end if;
      if aload='1' then
        a <= a_in;
      end if;
      if spush='1' then
        as_stack(sp1) <= t;
        sp <= sp + 1;
        sp1 <= sp1 + 1;
      elsif spopp='1' then
        sp <= sp - 1;
        sp1 <= sp1 - 1;
      end if;
      if rpush='1' then
        ar_stack(rp1) <= r_in;
        rp <= rp + 1;
        rp1 <= rp1 + 1;
      elsif rpopp='1' then
        rp <= rp - 1;
        rp1 <= rp1 - 1;
      end if;
      if uload='1' then
        uart_out <= t(0);
      end if;
    end if;
  end process sync;

end archcpu16;



