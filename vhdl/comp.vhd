-- Component declarations for the components in cyberdefs.vhd

  component inv
    port (
      i  : in  std_logic;                     -- input
      o  : out std_logic);                    -- output
  end component;
  component inv2
    port (
      i  : in  std_logic;                     -- input
      o  : out std_logic);                    -- output
  end component;
  component g2
    port (
      a, b : in  std_logic;                   -- inputs
      x    : out std_logic);                  -- output
  end component;
  component g3
    port (
      a, b, c : in  std_logic;                -- inputs
      x    : out std_logic);                  -- output
  end component;
  component g4
    port (
      a, b, c, d : in  std_logic;             -- inputs
      x    : out std_logic);                  -- output
  end component;
  component g5
    port (
      a, b, c, d, e : in  std_logic;          -- inputs
      x    : out std_logic);                  -- output
  end component;
  component g6
    port (
      a, b, c, d, e, f : in  std_logic;       -- inputs
      x    : out std_logic);                  -- output
  end component;
  component cxdriver
    port (
      a : in  std_logic;                      -- source
      x : out coaxsig);                       -- destination
  end component;
  component cxreceiver
    port (
      a : in  coaxsig;                        -- source
      x : out std_logic);                     -- destination
  end component;
  component rsflop
    port (
      s, r  : in  std_logic;                  -- set, reset
      s2, s3, s4, r2, r3, r4  : in  std_logic := '1';-- extra set, reset if needed
      q, qb : out std_logic);                 -- q and q.bar
  end component;
  component latch
    port (
      d, clk : in  std_logic;                 -- data (set), clock
      q, qb  : out std_logic);                -- q and q.bar
  end component;
  
