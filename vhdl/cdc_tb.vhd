-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Authors: Paul Koning, Dave Redell
--
-- Based on the original design by Seymour Cray and his team
--
-------------------------------------------------------------------------------

--  A testbench has no ports.

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;
use std.textio.all;
use work.sigs.all;

entity cdc_tb is
end cdc_tb;

architecture behav of cdc_tb is
   --  Declaration of the component that will be instantiated.
  component cdc6600 
    port (
      clk1, clk2, clk3, clk4 : in std_logic;
      w02_90 : in coaxsig;
      w02_900 : in coaxsig;
      w02_901 : in coaxsig;
      w02_902 : in coaxsig;
      w02_903 : in coaxsig;
      w02_904 : in coaxsig;
      w02_905 : in coaxsig;
      w02_906 : in coaxsig;
      w02_907 : in coaxsig;
      w02_91 : in coaxsig;
      w02_92 : in coaxsig;
      w02_93 : in coaxsig;
      w02_94 : in coaxsig;
      w02_95 : in coaxsig;
      w02_96 : in coaxsig;
      w02_97 : in coaxsig;
      w02_98 : in coaxsig;
      w02_99 : in coaxsig;
      w03_900 : out coaxsig;
      w03_901 : out coaxsig;
      w03_902 : out coaxsig;
      w03_903 : out coaxsig;
      w03_904 : out coaxsig;
      w03_905 : out coaxsig;
      w03_906 : out coaxsig;
      w03_907 : out coaxsig;
      w03_908 : out coaxsig;
      w03_91 : out coaxsig;
      w03_92 : out coaxsig;
      w03_93 : out coaxsig;
      w03_94 : out coaxsig;
      w03_95 : out coaxsig;
      w03_96 : out coaxsig;
      w03_97 : out coaxsig;
      w03_98 : out coaxsig;
      w03_99 : out coaxsig;
      w04_90 : in coaxsig;
      w04_900 : in coaxsig;
      w04_901 : in coaxsig;
      w04_902 : in coaxsig;
      w04_903 : in coaxsig;
      w04_904 : in coaxsig;
      w04_905 : inout misc;
      w04_91 : in coaxsig;
      w04_92 : in coaxsig;
      w04_93 : in coaxsig;
      w04_94 : in coaxsig;
      w04_95 : in coaxsig;
      w04_96 : in coaxsig;
      w04_97 : in coaxsig;
      w04_98 : in coaxsig;
      w04_99 : in coaxsig;
      w05_90 : in coaxsig;
      w05_900 : in coaxsig;
      w05_901 : in coaxsig;
      w05_902 : in coaxsig;
      w05_903 : in coaxsig;
      w05_904 : in coaxsig;
      w05_905 : in coaxsig;
      w05_906 : in coaxsig;
      w05_907 : inout misc;
      w05_91 : in coaxsig;
      w05_92 : in coaxsig;
      w05_93 : in coaxsig;
      w05_94 : in coaxsig;
      w05_95 : in coaxsig;
      w05_96 : in coaxsig;
      w05_97 : in coaxsig;
      w05_98 : in coaxsig;
      w05_99 : in coaxsig;
      w06_905 : in coaxsig;
      w06_90 : in coaxsig;
      w06_900 : in coaxsig;
      w06_901 : in coaxsig;
      w06_902 : in coaxsig;
      w06_903 : in coaxsig;
      w06_904 : in coaxsig;
      w06_906 : in coaxsig;
      w06_907 : in coaxsig;
      w06_91 : in coaxsig;
      w06_92 : in coaxsig;
      w06_93 : in coaxsig;
      w06_94 : in coaxsig;
      w06_95 : in coaxsig;
      w06_96 : in coaxsig;
      w06_97 : inout misc;
      w07_90 : in coaxsig;
      w07_900 : in coaxsig;
      w07_901 : in coaxsig;
      w07_902 : in coaxsig;
      w07_903 : in coaxsig;
      w07_904 : in coaxsig;
      w07_905 : in coaxsig;
      w07_906 : in coaxsig;
      w07_907 : in coaxsig;
      w07_91 : in coaxsig;
      w07_92 : in coaxsig;
      w07_93 : in coaxsig;
      w07_94 : in coaxsig;
      w07_95 : in coaxsig;
      w07_96 : in coaxsig;
      w07_97 : inout misc;
      w08_90 : out coaxsig;
      w08_900 : out coaxsig;
      w08_901 : out coaxsig;
      w08_902 : out coaxsig;
      w08_903 : out coaxsig;
      w08_904 : out coaxsig;
      w08_905 : in coaxsig;
      w08_91 : out coaxsig;
      w08_92 : out coaxsig;
      w08_93 : out coaxsig;
      w08_94 : out coaxsig;
      w08_95 : out coaxsig;
      w08_96 : out coaxsig;
      w08_97 : out coaxsig;
      w08_98 : out coaxsig;
      w08_99 : out coaxsig;
      w09_90 : out coaxsig;
      w09_900 : out coaxsig;
      w09_901 : out coaxsig;
      w09_902 : out coaxsig;
      w09_903 : out coaxsig;
      w09_904 : out coaxsig;
      w09_91 : out coaxsig;
      w09_92 : out coaxsig;
      w09_93 : out coaxsig;
      w09_94 : out coaxsig;
      w09_95 : out coaxsig;
      w09_96 : out coaxsig;
      w09_97 : out coaxsig;
      w09_98 : out coaxsig;
      w09_99 : out coaxsig;
      w10_90 : out coaxsig;
      w10_900 : out coaxsig;
      w10_901 : out coaxsig;
      w10_902 : out coaxsig;
      w10_903 : out coaxsig;
      w10_904 : out coaxsig;
      w10_91 : out coaxsig;
      w10_92 : out coaxsig;
      w10_93 : out coaxsig;
      w10_94 : out coaxsig;
      w10_95 : out coaxsig;
      w10_96 : out coaxsig;
      w10_97 : out coaxsig;
      w10_98 : out coaxsig;
      w10_99 : out coaxsig;
      w11_90 : out coaxsig;
      w11_900 : out coaxsig;
      w11_901 : out coaxsig;
      w11_902 : out coaxsig;
      w11_903 : out coaxsig;
      w11_904 : out coaxsig;
      w11_91 : out coaxsig;
      w11_92 : out coaxsig;
      w11_93 : out coaxsig;
      w11_94 : out coaxsig;
      w11_95 : out coaxsig;
      w11_96 : out coaxsig;
      w11_97 : out coaxsig;
      w11_98 : out coaxsig;
      w11_99 : out coaxsig;
      w12_900 : in coaxsig;
      w12_901 : out coaxsig;
      w12_902 : out coaxsig;
      w12_903 : out coaxsig;
      w12_904 : out coaxsig;
      w12_905 : out coaxsig;
      -- w13_90 : in coaxsig;
      -- w13_900 : in coaxsig;
      -- w13_901 : in coaxsig;
      -- w13_902 : in coaxsig;
      -- w13_903 : in coaxsig;
      -- w13_904 : in coaxsig;
      -- w13_905 : in coaxsig;
      -- w13_906 : out coaxsig;
      -- w13_907 : out coaxsig;
      -- w13_91 : in coaxsig;
      -- w13_92 : in coaxsig;
      -- w13_93 : in coaxsig;
      -- w13_94 : in coaxsig;
      -- w13_95 : in coaxsig;
      -- w13_96 : in coaxsig;
      -- w13_97 : in coaxsig;
      -- w13_98 : in coaxsig;
      -- w13_99 : in coaxsig;
      -- w14_90 : out coaxsig;
      -- w14_900 : out coaxsig;
      -- w14_901 : out coaxsig;
      -- w14_902 : out coaxsig;
      -- w14_903 : out coaxsig;
      -- w14_904 : out coaxsig;
      -- w14_905 : out coaxsig;
      -- w14_906 : out coaxsig;
      -- w14_907 : out coaxsig;
      -- w14_91 : out coaxsig;
      -- w14_92 : out coaxsig;
      -- w14_93 : out coaxsig;
      -- w14_94 : out coaxsig;
      -- w14_95 : out coaxsig;
      -- w14_96 : out coaxsig;
      -- w14_97 : out coaxsig;
      -- w14_98 : out coaxsig;
      -- w14_99 : out coaxsig;
      w15_90 : in coaxsig;
      w15_900 : in coaxsig;
      w15_901 : in coaxsig;
      w15_902 : in coaxsig;
      w15_903 : in coaxsig;
      w15_904 : in coaxsig;
      w15_905 : in coaxsig;
      w15_906 : out coaxsig;
      w15_907 : out coaxsig;
      w15_91 : in coaxsig;
      w15_92 : in coaxsig;
      w15_93 : in coaxsig;
      w15_94 : in coaxsig;
      w15_95 : in coaxsig;
      w15_96 : in coaxsig;
      w15_97 : in coaxsig;
      w15_98 : in coaxsig;
      w15_99 : in coaxsig;
      w16_90 : out coaxsig;
      w16_900 : out coaxsig;
      w16_901 : out coaxsig;
      w16_902 : out coaxsig;
      w16_903 : out coaxsig;
      w16_904 : out coaxsig;
      w16_905 : out coaxsig;
      w16_906 : out coaxsig;
      w16_907 : out coaxsig;
      w16_91 : out coaxsig;
      w16_92 : out coaxsig;
      w16_93 : out coaxsig;
      w16_94 : out coaxsig;
      w16_95 : out coaxsig;
      w16_96 : out coaxsig;
      w16_97 : out coaxsig;
      w16_98 : out coaxsig;
      w16_99 : out coaxsig;
      w17_90 : in coaxsig;
      w17_900 : in coaxsig;
      w17_901 : in coaxsig;
      w17_902 : in coaxsig;
      w17_903 : in coaxsig;
      w17_904 : in coaxsig;
      w17_905 : in coaxsig;
      w17_906 : out coaxsig;
      w17_907 : out coaxsig;
      w17_91 : in coaxsig;
      w17_92 : in coaxsig;
      w17_93 : in coaxsig;
      w17_94 : in coaxsig;
      w17_95 : in coaxsig;
      w17_96 : in coaxsig;
      w17_97 : in coaxsig;
      w17_98 : in coaxsig;
      w17_99 : in coaxsig;
      w18_90 : out coaxsig;
      w18_900 : out coaxsig;
      w18_901 : out coaxsig;
      w18_902 : out coaxsig;
      w18_903 : out coaxsig;
      w18_904 : out coaxsig;
      w18_905 : out coaxsig;
      w18_906 : out coaxsig;
      w18_907 : out coaxsig;
      w18_91 : out coaxsig;
      w18_92 : out coaxsig;
      w18_93 : out coaxsig;
      w18_94 : out coaxsig;
      w18_95 : out coaxsig;
      w18_96 : out coaxsig;
      w18_97 : out coaxsig;
      w18_98 : out coaxsig;
      w18_99 : out coaxsig;
      w19_90 : in coaxsig;
      w19_900 : in coaxsig;
      w19_901 : in coaxsig;
      w19_902 : in coaxsig;
      w19_903 : in coaxsig;
      w19_904 : in coaxsig;
      w19_905 : in coaxsig;
      w19_906 : out coaxsig;
      w19_907 : out coaxsig;
      w19_91 : in coaxsig;
      w19_92 : in coaxsig;
      w19_93 : in coaxsig;
      w19_94 : in coaxsig;
      w19_95 : in coaxsig;
      w19_96 : in coaxsig;
      w19_97 : in coaxsig;
      w19_98 : in coaxsig;
      w19_99 : in coaxsig;
      w20_90 : out coaxsig;
      w20_900 : out coaxsig;
      w20_901 : out coaxsig;
      w20_902 : out coaxsig;
      w20_903 : out coaxsig;
      w20_904 : out coaxsig;
      w20_905 : out coaxsig;
      w20_906 : out coaxsig;
      w20_907 : out coaxsig;
      w20_91 : out coaxsig;
      w20_92 : out coaxsig;
      w20_93 : out coaxsig;
      w20_94 : out coaxsig;
      w20_95 : out coaxsig;
      w20_96 : out coaxsig;
      w20_97 : out coaxsig;
      w20_98 : out coaxsig;
      w20_99 : out coaxsig;
      w21_90 : in coaxsig;
      w21_900 : in coaxsig;
      w21_901 : in coaxsig;
      w21_902 : in coaxsig;
      w21_903 : in coaxsig;
      w21_904 : in coaxsig;
      w21_905 : in coaxsig;
      w21_906 : out coaxsig;
      w21_907 : out coaxsig;
      w21_91 : in coaxsig;
      w21_92 : in coaxsig;
      w21_93 : in coaxsig;
      w21_94 : in coaxsig;
      w21_95 : in coaxsig;
      w21_96 : in coaxsig;
      w21_97 : in coaxsig;
      w21_98 : in coaxsig;
      w21_99 : in coaxsig;
      w22_90 : out coaxsig;
      w22_900 : out coaxsig;
      w22_901 : out coaxsig;
      w22_902 : out coaxsig;
      w22_903 : out coaxsig;
      w22_904 : out coaxsig;
      w22_905 : out coaxsig;
      w22_906 : out coaxsig;
      w22_907 : out coaxsig;
      w22_91 : out coaxsig;
      w22_92 : out coaxsig;
      w22_93 : out coaxsig;
      w22_94 : out coaxsig;
      w22_95 : out coaxsig;
      w22_96 : out coaxsig;
      w22_97 : out coaxsig;
      w22_98 : out coaxsig;
      w22_99 : out coaxsig;
      w23_90 : in coaxsig;
      w23_900 : in coaxsig;
      w23_901 : in coaxsig;
      w23_902 : in coaxsig;
      w23_903 : in coaxsig;
      w23_904 : in coaxsig;
      w23_905 : in coaxsig;
      w23_906 : out coaxsig;
      w23_907 : out coaxsig;
      w23_91 : in coaxsig;
      w23_92 : in coaxsig;
      w23_93 : in coaxsig;
      w23_94 : in coaxsig;
      w23_95 : in coaxsig;
      w23_96 : in coaxsig;
      w23_97 : in coaxsig;
      w23_98 : in coaxsig;
      w23_99 : in coaxsig;
      w24_90 : out coaxsig;
      w24_900 : out coaxsig;
      w24_901 : out coaxsig;
      w24_902 : out coaxsig;
      w24_903 : out coaxsig;
      w24_904 : out coaxsig;
      w24_905 : out coaxsig;
      w24_906 : out coaxsig;
      w24_907 : out coaxsig;
      w24_91 : out coaxsig;
      w24_92 : out coaxsig;
      w24_93 : out coaxsig;
      w24_94 : out coaxsig;
      w24_95 : out coaxsig;
      w24_96 : out coaxsig;
      w24_97 : out coaxsig;
      w24_98 : out coaxsig;
      w24_99 : out coaxsig;
      w25_90 : in coaxsig;
      w25_900 : in coaxsig;
      w25_901 : in coaxsig;
      w25_902 : in coaxsig;
      w25_903 : in coaxsig;
      w25_904 : in coaxsig;
      w25_905 : in coaxsig;
      w25_906 : out coaxsig;
      w25_907 : out coaxsig;
      w25_91 : in coaxsig;
      w25_92 : in coaxsig;
      w25_93 : in coaxsig;
      w25_94 : in coaxsig;
      w25_95 : in coaxsig;
      w25_96 : in coaxsig;
      w25_97 : in coaxsig;
      w25_98 : in coaxsig;
      w25_99 : in coaxsig;
      w26_90 : out coaxsig;
      w26_900 : out coaxsig;
      w26_901 : out coaxsig;
      w26_902 : out coaxsig;
      w26_903 : out coaxsig;
      w26_904 : out coaxsig;
      w26_905 : out coaxsig;
      w26_906 : out coaxsig;
      w26_907 : out coaxsig;
      w26_91 : out coaxsig;
      w26_92 : out coaxsig;
      w26_93 : out coaxsig;
      w26_94 : out coaxsig;
      w26_95 : out coaxsig;
      w26_96 : out coaxsig;
      w26_97 : out coaxsig;
      w26_98 : out coaxsig;
      w26_99 : out coaxsig;
      w27_90 : out coaxsig;
      w27_900 : out coaxsig;
      w27_901 : out coaxsig;
      w27_902 : out coaxsig;
      w27_903 : out coaxsig;
      w27_904 : out coaxsig;
      w27_905 : out coaxsig;
      w27_906 : out coaxsig;
      w27_907 : out coaxsig;
      w27_91 : out coaxsig;
      w27_92 : out coaxsig;
      w27_93 : out coaxsig;
      w27_94 : out coaxsig;
      w27_95 : out coaxsig;
      w27_96 : out coaxsig;
      w27_97 : out coaxsig;
      w27_98 : out coaxsig;
      w27_99 : out coaxsig;
      w28_90 : in coaxsig;
      w28_900 : in coaxsig;
      w28_901 : in coaxsig;
      w28_902 : in coaxsig;
      w28_903 : in coaxsig;
      w28_904 : in coaxsig;
      w28_905 : in coaxsig;
      w28_906 : out coaxsig;
      w28_907 : out coaxsig;
      w28_91 : in coaxsig;
      w28_92 : in coaxsig;
      w28_93 : in coaxsig;
      w28_94 : in coaxsig;
      w28_95 : in coaxsig;
      w28_96 : in coaxsig;
      w28_97 : in coaxsig;
      w28_98 : in coaxsig;
      w28_99 : in coaxsig;
      w31_90 : in coaxsig;
      w31_900 : in coaxsig;
      w31_901 : in coaxsig;
      w31_902 : in coaxsig;
      w31_903 : in coaxsig;
      w31_904 : in coaxsig;
      w31_905 : in coaxsig;
      w31_906 : out coaxsig;
      w31_907 : out coaxsig;
      w31_91 : in coaxsig;
      w31_92 : in coaxsig;
      w31_93 : in coaxsig;
      w31_94 : in coaxsig;
      w31_95 : in coaxsig;
      w31_96 : in coaxsig;
      w31_97 : in coaxsig;
      w31_98 : in coaxsig;
      w31_99 : in coaxsig;
      w32_90 : out coaxsig;
      w32_900 : out coaxsig;
      w32_901 : out coaxsig;
      w32_902 : out coaxsig;
      w32_903 : out coaxsig;
      w32_904 : out coaxsig;
      w32_905 : out coaxsig;
      w32_906 : out coaxsig;
      w32_907 : out coaxsig;
      w32_91 : out coaxsig;
      w32_92 : out coaxsig;
      w32_93 : out coaxsig;
      w32_94 : out coaxsig;
      w32_95 : out coaxsig;
      w32_96 : out coaxsig;
      w32_97 : out coaxsig;
      w32_98 : out coaxsig;
      w32_99 : out coaxsig;
      w33_90 : in coaxsig;
      w33_900 : in coaxsig;
      w33_901 : in coaxsig;
      w33_902 : in coaxsig;
      w33_903 : in coaxsig;
      w33_904 : in coaxsig;
      w33_905 : in coaxsig;
      w33_906 : out coaxsig;
      w33_907 : out coaxsig;
      w33_91 : in coaxsig;
      w33_92 : in coaxsig;
      w33_93 : in coaxsig;
      w33_94 : in coaxsig;
      w33_95 : in coaxsig;
      w33_96 : in coaxsig;
      w33_97 : in coaxsig;
      w33_98 : in coaxsig;
      w33_99 : in coaxsig;
      w34_90 : out coaxsig;
      w34_900 : out coaxsig;
      w34_901 : out coaxsig;
      w34_902 : out coaxsig;
      w34_903 : out coaxsig;
      w34_904 : out coaxsig;
      w34_905 : out coaxsig;
      w34_906 : out coaxsig;
      w34_907 : out coaxsig;
      w34_91 : out coaxsig;
      w34_92 : out coaxsig;
      w34_93 : out coaxsig;
      w34_94 : out coaxsig;
      w34_95 : out coaxsig;
      w34_96 : out coaxsig;
      w34_97 : out coaxsig;
      w34_98 : out coaxsig;
      w34_99 : out coaxsig;
      w35_90 : in coaxsig;
      w35_900 : in coaxsig;
      w35_901 : in coaxsig;
      w35_902 : in coaxsig;
      w35_903 : in coaxsig;
      w35_904 : in coaxsig;
      w35_905 : in coaxsig;
      w35_906 : out coaxsig;
      w35_907 : out coaxsig;
      w35_91 : in coaxsig;
      w35_92 : in coaxsig;
      w35_93 : in coaxsig;
      w35_94 : in coaxsig;
      w35_95 : in coaxsig;
      w35_96 : in coaxsig;
      w35_97 : in coaxsig;
      w35_98 : in coaxsig;
      w35_99 : in coaxsig;
      w36_90 : out coaxsig;
      w36_900 : out coaxsig;
      w36_901 : out coaxsig;
      w36_902 : out coaxsig;
      w36_903 : out coaxsig;
      w36_904 : out coaxsig;
      w36_905 : out coaxsig;
      w36_906 : out coaxsig;
      w36_907 : out coaxsig;
      w36_91 : out coaxsig;
      w36_92 : out coaxsig;
      w36_93 : out coaxsig;
      w36_94 : out coaxsig;
      w36_95 : out coaxsig;
      w36_96 : out coaxsig;
      w36_97 : out coaxsig;
      w36_98 : out coaxsig;
      w36_99 : out coaxsig;
      w999_90 : in std_logic;
      w999_91 : in std_logic;
      w999_92 : in std_logic);
  end component;
  signal clk1 : std_logic := '1';        -- clock phase 1
  signal clk2, clk3, clk4 : std_logic := '0';  -- clock phase 2-4
  type testvec is array (1 to 80) of std_logic;
  signal coax1 : coaxsigs;
  signal w02_90 : coaxsig := '0';
  signal w02_900 : coaxsig := '0';
  signal w02_901 : coaxsig := '0';
  signal w02_902 : coaxsig := '0';
  signal w02_903 : coaxsig := '0';
  signal w02_904 : coaxsig := '0';
  signal w02_905 : coaxsig := '0';
  signal w02_906 : coaxsig := '0';
  signal w02_907 : coaxsig := '0';
  signal w02_91 : coaxsig := '0';
  signal w02_92 : coaxsig := '0';
  signal w02_93 : coaxsig := '0';
  signal w02_94 : coaxsig := '0';
  signal w02_95 : coaxsig := '0';
  signal w02_96 : coaxsig := '0';
  signal w02_97 : coaxsig := '0';
  signal w02_98 : coaxsig := '0';
  signal w02_99 : coaxsig := '0';
  signal w03_900 : coaxsig := '0';
  signal w03_901 : coaxsig := '0';
  signal w03_902 : coaxsig := '0';
  signal w03_903 : coaxsig := '0';
  signal w03_904 : coaxsig := '0';
  signal w03_905 : coaxsig := '0';
  signal w03_906 : coaxsig := '0';
  signal w03_907 : coaxsig := '0';
  signal w03_908 : coaxsig := '0';
  signal w03_91 : coaxsig := '0';
  signal w03_92 : coaxsig := '0';
  signal w03_93 : coaxsig := '0';
  signal w03_94 : coaxsig := '0';
  signal w03_95 : coaxsig := '0';
  signal w03_96 : coaxsig := '0';
  signal w03_97 : coaxsig := '0';
  signal w03_98 : coaxsig := '0';
  signal w03_99 : coaxsig := '0';
  signal w04_90 : coaxsig := '0';
  signal w04_900 : coaxsig := '0';
  signal w04_901 : coaxsig := '0';
  signal w04_902 : coaxsig := '0';
  signal w04_903 : coaxsig := '0';
  signal w04_904 : coaxsig := '0';
  signal w04_905 : misc := '0';
  signal w04_91 : coaxsig := '0';
  signal w04_92 : coaxsig := '0';
  signal w04_93 : coaxsig := '0';
  signal w04_94 : coaxsig := '0';
  signal w04_95 : coaxsig := '0';
  signal w04_96 : coaxsig := '0';
  signal w04_97 : coaxsig := '0';
  signal w04_98 : coaxsig := '0';
  signal w04_99 : coaxsig := '0';
  signal w05_90 : coaxsig := '0';
  signal w05_900 : coaxsig := '0';
  signal w05_901 : coaxsig := '0';
  signal w05_902 : coaxsig := '0';
  signal w05_903 : coaxsig := '0';
  signal w05_904 : coaxsig := '0';
  signal w05_905 : coaxsig := '0';
  signal w05_906 : coaxsig := '0';
  signal w05_907 : misc := '0';
  signal w05_91 : coaxsig := '0';
  signal w05_92 : coaxsig := '0';
  signal w05_93 : coaxsig := '0';
  signal w05_94 : coaxsig := '0';
  signal w05_95 : coaxsig := '0';
  signal w05_96 : coaxsig := '0';
  signal w05_97 : coaxsig := '0';
  signal w05_98 : coaxsig := '0';
  signal w05_99 : coaxsig := '0';
  signal w06_905 : coaxsig := '0';
  signal w06_90 : coaxsig := '0';
  signal w06_900 : coaxsig := '0';
  signal w06_901 : coaxsig := '0';
  signal w06_902 : coaxsig := '0';
  signal w06_903 : coaxsig := '0';
  signal w06_904 : coaxsig := '0';
  signal w06_906 : coaxsig := '0';
  signal w06_907 : coaxsig := '0';
  signal w06_91 : coaxsig := '0';
  signal w06_92 : coaxsig := '0';
  signal w06_93 : coaxsig := '0';
  signal w06_94 : coaxsig := '0';
  signal w06_95 : coaxsig := '0';
  signal w06_96 : coaxsig := '0';
  signal w06_97 : misc := '0';
  signal w07_90 : coaxsig := '0';
  signal w07_900 : coaxsig := '0';
  signal w07_901 : coaxsig := '0';
  signal w07_902 : coaxsig := '0';
  signal w07_903 : coaxsig := '0';
  signal w07_904 : coaxsig := '0';
  signal w07_905 : coaxsig := '0';
  signal w07_906 : coaxsig := '0';
  signal w07_907 : coaxsig := '0';
  signal w07_91 : coaxsig := '0';
  signal w07_92 : coaxsig := '0';
  signal w07_93 : coaxsig := '0';
  signal w07_94 : coaxsig := '0';
  signal w07_95 : coaxsig := '0';
  signal w07_96 : coaxsig := '0';
  signal w07_97 : misc := '0';
  signal w08_90 : coaxsig := '0';
  signal w08_900 : coaxsig := '0';
  signal w08_901 : coaxsig := '0';
  signal w08_902 : coaxsig := '0';
  signal w08_903 : coaxsig := '0';
  signal w08_904 : coaxsig := '0';
  signal w08_905 : coaxsig := '0';
  signal w08_91 : coaxsig := '0';
  signal w08_92 : coaxsig := '0';
  signal w08_93 : coaxsig := '0';
  signal w08_94 : coaxsig := '0';
  signal w08_95 : coaxsig := '0';
  signal w08_96 : coaxsig := '0';
  signal w08_97 : coaxsig := '0';
  signal w08_98 : coaxsig := '0';
  signal w08_99 : coaxsig := '0';
  signal w09_90 : coaxsig := '0';
  signal w09_900 : coaxsig := '0';
  signal w09_901 : coaxsig := '0';
  signal w09_902 : coaxsig := '0';
  signal w09_903 : coaxsig := '0';
  signal w09_904 : coaxsig := '0';
  signal w09_91 : coaxsig := '0';
  signal w09_92 : coaxsig := '0';
  signal w09_93 : coaxsig := '0';
  signal w09_94 : coaxsig := '0';
  signal w09_95 : coaxsig := '0';
  signal w09_96 : coaxsig := '0';
  signal w09_97 : coaxsig := '0';
  signal w09_98 : coaxsig := '0';
  signal w09_99 : coaxsig := '0';
  signal w10_90 : coaxsig := '0';
  signal w10_900 : coaxsig := '0';
  signal w10_901 : coaxsig := '0';
  signal w10_902 : coaxsig := '0';
  signal w10_903 : coaxsig := '0';
  signal w10_904 : coaxsig := '0';
  signal w10_91 : coaxsig := '0';
  signal w10_92 : coaxsig := '0';
  signal w10_93 : coaxsig := '0';
  signal w10_94 : coaxsig := '0';
  signal w10_95 : coaxsig := '0';
  signal w10_96 : coaxsig := '0';
  signal w10_97 : coaxsig := '0';
  signal w10_98 : coaxsig := '0';
  signal w10_99 : coaxsig := '0';
  signal w11_90 : coaxsig := '0';
  signal w11_900 : coaxsig := '0';
  signal w11_901 : coaxsig := '0';
  signal w11_902 : coaxsig := '0';
  signal w11_903 : coaxsig := '0';
  signal w11_904 : coaxsig := '0';
  signal w11_91 : coaxsig := '0';
  signal w11_92 : coaxsig := '0';
  signal w11_93 : coaxsig := '0';
  signal w11_94 : coaxsig := '0';
  signal w11_95 : coaxsig := '0';
  signal w11_96 : coaxsig := '0';
  signal w11_97 : coaxsig := '0';
  signal w11_98 : coaxsig := '0';
  signal w11_99 : coaxsig := '0';
  signal w12_900 : coaxsig := '0';
  signal w12_901 : coaxsig := '0';
  signal w12_902 : coaxsig := '0';
  signal w12_903 : coaxsig := '0';
  signal w12_904 : coaxsig := '0';
  signal w12_905 : coaxsig := '0';
  -- signal w13_90 : coaxsig := '0';
  -- signal w13_900 : coaxsig := '0';
  -- signal w13_901 : coaxsig := '0';
  -- signal w13_902 : coaxsig := '0';
  -- signal w13_903 : coaxsig := '0';
  -- signal w13_904 : coaxsig := '0';
  -- signal w13_905 : coaxsig := '0';
  -- signal w13_906 : coaxsig := '0';
  -- signal w13_907 : coaxsig := '0';
  -- signal w13_91 : coaxsig := '0';
  -- signal w13_92 : coaxsig := '0';
  -- signal w13_93 : coaxsig := '0';
  -- signal w13_94 : coaxsig := '0';
  -- signal w13_95 : coaxsig := '0';
  -- signal w13_96 : coaxsig := '0';
  -- signal w13_97 : coaxsig := '0';
  -- signal w13_98 : coaxsig := '0';
  -- signal w13_99 : coaxsig := '0';
  -- signal w14_90 : coaxsig := '0';
  -- signal w14_900 : coaxsig := '0';
  -- signal w14_901 : coaxsig := '0';
  -- signal w14_902 : coaxsig := '0';
  -- signal w14_903 : coaxsig := '0';
  -- signal w14_904 : coaxsig := '0';
  -- signal w14_905 : coaxsig := '0';
  -- signal w14_906 : coaxsig := '0';
  -- signal w14_907 : coaxsig := '0';
  -- signal w14_91 : coaxsig := '0';
  -- signal w14_92 : coaxsig := '0';
  -- signal w14_93 : coaxsig := '0';
  -- signal w14_94 : coaxsig := '0';
  -- signal w14_95 : coaxsig := '0';
  -- signal w14_96 : coaxsig := '0';
  -- signal w14_97 : coaxsig := '0';
  -- signal w14_98 : coaxsig := '0';
  -- signal w14_99 : coaxsig := '0';
  signal w15_90 : coaxsig := '0';
  signal w15_900 : coaxsig := '0';
  signal w15_901 : coaxsig := '0';
  signal w15_902 : coaxsig := '0';
  signal w15_903 : coaxsig := '0';
  signal w15_904 : coaxsig := '0';
  signal w15_905 : coaxsig := '0';
  signal w15_906 : coaxsig := '0';
  signal w15_907 : coaxsig := '0';
  signal w15_91 : coaxsig := '0';
  signal w15_92 : coaxsig := '0';
  signal w15_93 : coaxsig := '0';
  signal w15_94 : coaxsig := '0';
  signal w15_95 : coaxsig := '0';
  signal w15_96 : coaxsig := '0';
  signal w15_97 : coaxsig := '0';
  signal w15_98 : coaxsig := '0';
  signal w15_99 : coaxsig := '0';
  signal w16_90 : coaxsig := '0';
  signal w16_900 : coaxsig := '0';
  signal w16_901 : coaxsig := '0';
  signal w16_902 : coaxsig := '0';
  signal w16_903 : coaxsig := '0';
  signal w16_904 : coaxsig := '0';
  signal w16_905 : coaxsig := '0';
  signal w16_906 : coaxsig := '0';
  signal w16_907 : coaxsig := '0';
  signal w16_91 : coaxsig := '0';
  signal w16_92 : coaxsig := '0';
  signal w16_93 : coaxsig := '0';
  signal w16_94 : coaxsig := '0';
  signal w16_95 : coaxsig := '0';
  signal w16_96 : coaxsig := '0';
  signal w16_97 : coaxsig := '0';
  signal w16_98 : coaxsig := '0';
  signal w16_99 : coaxsig := '0';
  signal w17_90 : coaxsig := '0';
  signal w17_900 : coaxsig := '0';
  signal w17_901 : coaxsig := '0';
  signal w17_902 : coaxsig := '0';
  signal w17_903 : coaxsig := '0';
  signal w17_904 : coaxsig := '0';
  signal w17_905 : coaxsig := '0';
  signal w17_906 : coaxsig := '0';
  signal w17_907 : coaxsig := '0';
  signal w17_91 : coaxsig := '0';
  signal w17_92 : coaxsig := '0';
  signal w17_93 : coaxsig := '0';
  signal w17_94 : coaxsig := '0';
  signal w17_95 : coaxsig := '0';
  signal w17_96 : coaxsig := '0';
  signal w17_97 : coaxsig := '0';
  signal w17_98 : coaxsig := '0';
  signal w17_99 : coaxsig := '0';
  signal w18_90 : coaxsig := '0';
  signal w18_900 : coaxsig := '0';
  signal w18_901 : coaxsig := '0';
  signal w18_902 : coaxsig := '0';
  signal w18_903 : coaxsig := '0';
  signal w18_904 : coaxsig := '0';
  signal w18_905 : coaxsig := '0';
  signal w18_906 : coaxsig := '0';
  signal w18_907 : coaxsig := '0';
  signal w18_91 : coaxsig := '0';
  signal w18_92 : coaxsig := '0';
  signal w18_93 : coaxsig := '0';
  signal w18_94 : coaxsig := '0';
  signal w18_95 : coaxsig := '0';
  signal w18_96 : coaxsig := '0';
  signal w18_97 : coaxsig := '0';
  signal w18_98 : coaxsig := '0';
  signal w18_99 : coaxsig := '0';
  signal w19_90 : coaxsig := '0';
  signal w19_900 : coaxsig := '0';
  signal w19_901 : coaxsig := '0';
  signal w19_902 : coaxsig := '0';
  signal w19_903 : coaxsig := '0';
  signal w19_904 : coaxsig := '0';
  signal w19_905 : coaxsig := '0';
  signal w19_906 : coaxsig := '0';
  signal w19_907 : coaxsig := '0';
  signal w19_91 : coaxsig := '0';
  signal w19_92 : coaxsig := '0';
  signal w19_93 : coaxsig := '0';
  signal w19_94 : coaxsig := '0';
  signal w19_95 : coaxsig := '0';
  signal w19_96 : coaxsig := '0';
  signal w19_97 : coaxsig := '0';
  signal w19_98 : coaxsig := '0';
  signal w19_99 : coaxsig := '0';
  signal w20_90 : coaxsig := '0';
  signal w20_900 : coaxsig := '0';
  signal w20_901 : coaxsig := '0';
  signal w20_902 : coaxsig := '0';
  signal w20_903 : coaxsig := '0';
  signal w20_904 : coaxsig := '0';
  signal w20_905 : coaxsig := '0';
  signal w20_906 : coaxsig := '0';
  signal w20_907 : coaxsig := '0';
  signal w20_91 : coaxsig := '0';
  signal w20_92 : coaxsig := '0';
  signal w20_93 : coaxsig := '0';
  signal w20_94 : coaxsig := '0';
  signal w20_95 : coaxsig := '0';
  signal w20_96 : coaxsig := '0';
  signal w20_97 : coaxsig := '0';
  signal w20_98 : coaxsig := '0';
  signal w20_99 : coaxsig := '0';
  signal w21_90 : coaxsig := '0';
  signal w21_900 : coaxsig := '0';
  signal w21_901 : coaxsig := '0';
  signal w21_902 : coaxsig := '0';
  signal w21_903 : coaxsig := '0';
  signal w21_904 : coaxsig := '0';
  signal w21_905 : coaxsig := '0';
  signal w21_906 : coaxsig := '0';
  signal w21_907 : coaxsig := '0';
  signal w21_91 : coaxsig := '0';
  signal w21_92 : coaxsig := '0';
  signal w21_93 : coaxsig := '0';
  signal w21_94 : coaxsig := '0';
  signal w21_95 : coaxsig := '0';
  signal w21_96 : coaxsig := '0';
  signal w21_97 : coaxsig := '0';
  signal w21_98 : coaxsig := '0';
  signal w21_99 : coaxsig := '0';
  signal w22_90 : coaxsig := '0';
  signal w22_900 : coaxsig := '0';
  signal w22_901 : coaxsig := '0';
  signal w22_902 : coaxsig := '0';
  signal w22_903 : coaxsig := '0';
  signal w22_904 : coaxsig := '0';
  signal w22_905 : coaxsig := '0';
  signal w22_906 : coaxsig := '0';
  signal w22_907 : coaxsig := '0';
  signal w22_91 : coaxsig := '0';
  signal w22_92 : coaxsig := '0';
  signal w22_93 : coaxsig := '0';
  signal w22_94 : coaxsig := '0';
  signal w22_95 : coaxsig := '0';
  signal w22_96 : coaxsig := '0';
  signal w22_97 : coaxsig := '0';
  signal w22_98 : coaxsig := '0';
  signal w22_99 : coaxsig := '0';
  signal w23_90 : coaxsig := '0';
  signal w23_900 : coaxsig := '0';
  signal w23_901 : coaxsig := '0';
  signal w23_902 : coaxsig := '0';
  signal w23_903 : coaxsig := '0';
  signal w23_904 : coaxsig := '0';
  signal w23_905 : coaxsig := '0';
  signal w23_906 : coaxsig := '0';
  signal w23_907 : coaxsig := '0';
  signal w23_91 : coaxsig := '0';
  signal w23_92 : coaxsig := '0';
  signal w23_93 : coaxsig := '0';
  signal w23_94 : coaxsig := '0';
  signal w23_95 : coaxsig := '0';
  signal w23_96 : coaxsig := '0';
  signal w23_97 : coaxsig := '0';
  signal w23_98 : coaxsig := '0';
  signal w23_99 : coaxsig := '0';
  signal w24_90 : coaxsig := '0';
  signal w24_900 : coaxsig := '0';
  signal w24_901 : coaxsig := '0';
  signal w24_902 : coaxsig := '0';
  signal w24_903 : coaxsig := '0';
  signal w24_904 : coaxsig := '0';
  signal w24_905 : coaxsig := '0';
  signal w24_906 : coaxsig := '0';
  signal w24_907 : coaxsig := '0';
  signal w24_91 : coaxsig := '0';
  signal w24_92 : coaxsig := '0';
  signal w24_93 : coaxsig := '0';
  signal w24_94 : coaxsig := '0';
  signal w24_95 : coaxsig := '0';
  signal w24_96 : coaxsig := '0';
  signal w24_97 : coaxsig := '0';
  signal w24_98 : coaxsig := '0';
  signal w24_99 : coaxsig := '0';
  signal w25_90 : coaxsig := '0';
  signal w25_900 : coaxsig := '0';
  signal w25_901 : coaxsig := '0';
  signal w25_902 : coaxsig := '0';
  signal w25_903 : coaxsig := '0';
  signal w25_904 : coaxsig := '0';
  signal w25_905 : coaxsig := '0';
  signal w25_906 : coaxsig := '0';
  signal w25_907 : coaxsig := '0';
  signal w25_91 : coaxsig := '0';
  signal w25_92 : coaxsig := '0';
  signal w25_93 : coaxsig := '0';
  signal w25_94 : coaxsig := '0';
  signal w25_95 : coaxsig := '0';
  signal w25_96 : coaxsig := '0';
  signal w25_97 : coaxsig := '0';
  signal w25_98 : coaxsig := '0';
  signal w25_99 : coaxsig := '0';
  signal w26_90 : coaxsig := '0';
  signal w26_900 : coaxsig := '0';
  signal w26_901 : coaxsig := '0';
  signal w26_902 : coaxsig := '0';
  signal w26_903 : coaxsig := '0';
  signal w26_904 : coaxsig := '0';
  signal w26_905 : coaxsig := '0';
  signal w26_906 : coaxsig := '0';
  signal w26_907 : coaxsig := '0';
  signal w26_91 : coaxsig := '0';
  signal w26_92 : coaxsig := '0';
  signal w26_93 : coaxsig := '0';
  signal w26_94 : coaxsig := '0';
  signal w26_95 : coaxsig := '0';
  signal w26_96 : coaxsig := '0';
  signal w26_97 : coaxsig := '0';
  signal w26_98 : coaxsig := '0';
  signal w26_99 : coaxsig := '0';
  signal w27_90 : coaxsig := '0';
  signal w27_900 : coaxsig := '0';
  signal w27_901 : coaxsig := '0';
  signal w27_902 : coaxsig := '0';
  signal w27_903 : coaxsig := '0';
  signal w27_904 : coaxsig := '0';
  signal w27_905 : coaxsig := '0';
  signal w27_906 : coaxsig := '0';
  signal w27_907 : coaxsig := '0';
  signal w27_91 : coaxsig := '0';
  signal w27_92 : coaxsig := '0';
  signal w27_93 : coaxsig := '0';
  signal w27_94 : coaxsig := '0';
  signal w27_95 : coaxsig := '0';
  signal w27_96 : coaxsig := '0';
  signal w27_97 : coaxsig := '0';
  signal w27_98 : coaxsig := '0';
  signal w27_99 : coaxsig := '0';
  signal w28_90 : coaxsig := '0';
  signal w28_900 : coaxsig := '0';
  signal w28_901 : coaxsig := '0';
  signal w28_902 : coaxsig := '0';
  signal w28_903 : coaxsig := '0';
  signal w28_904 : coaxsig := '0';
  signal w28_905 : coaxsig := '0';
  signal w28_906 : coaxsig := '0';
  signal w28_907 : coaxsig := '0';
  signal w28_91 : coaxsig := '0';
  signal w28_92 : coaxsig := '0';
  signal w28_93 : coaxsig := '0';
  signal w28_94 : coaxsig := '0';
  signal w28_95 : coaxsig := '0';
  signal w28_96 : coaxsig := '0';
  signal w28_97 : coaxsig := '0';
  signal w28_98 : coaxsig := '0';
  signal w28_99 : coaxsig := '0';
  signal w31_90 : coaxsig := '0';
  signal w31_900 : coaxsig := '0';
  signal w31_901 : coaxsig := '0';
  signal w31_902 : coaxsig := '0';
  signal w31_903 : coaxsig := '0';
  signal w31_904 : coaxsig := '0';
  signal w31_905 : coaxsig := '0';
  signal w31_906 : coaxsig := '0';
  signal w31_907 : coaxsig := '0';
  signal w31_91 : coaxsig := '0';
  signal w31_92 : coaxsig := '0';
  signal w31_93 : coaxsig := '0';
  signal w31_94 : coaxsig := '0';
  signal w31_95 : coaxsig := '0';
  signal w31_96 : coaxsig := '0';
  signal w31_97 : coaxsig := '0';
  signal w31_98 : coaxsig := '0';
  signal w31_99 : coaxsig := '0';
  signal w32_90 : coaxsig := '0';
  signal w32_900 : coaxsig := '0';
  signal w32_901 : coaxsig := '0';
  signal w32_902 : coaxsig := '0';
  signal w32_903 : coaxsig := '0';
  signal w32_904 : coaxsig := '0';
  signal w32_905 : coaxsig := '0';
  signal w32_906 : coaxsig := '0';
  signal w32_907 : coaxsig := '0';
  signal w32_91 : coaxsig := '0';
  signal w32_92 : coaxsig := '0';
  signal w32_93 : coaxsig := '0';
  signal w32_94 : coaxsig := '0';
  signal w32_95 : coaxsig := '0';
  signal w32_96 : coaxsig := '0';
  signal w32_97 : coaxsig := '0';
  signal w32_98 : coaxsig := '0';
  signal w32_99 : coaxsig := '0';
  signal w33_90 : coaxsig := '0';
  signal w33_900 : coaxsig := '0';
  signal w33_901 : coaxsig := '0';
  signal w33_902 : coaxsig := '0';
  signal w33_903 : coaxsig := '0';
  signal w33_904 : coaxsig := '0';
  signal w33_905 : coaxsig := '0';
  signal w33_906 : coaxsig := '0';
  signal w33_907 : coaxsig := '0';
  signal w33_91 : coaxsig := '0';
  signal w33_92 : coaxsig := '0';
  signal w33_93 : coaxsig := '0';
  signal w33_94 : coaxsig := '0';
  signal w33_95 : coaxsig := '0';
  signal w33_96 : coaxsig := '0';
  signal w33_97 : coaxsig := '0';
  signal w33_98 : coaxsig := '0';
  signal w33_99 : coaxsig := '0';
  signal w34_90 : coaxsig := '0';
  signal w34_900 : coaxsig := '0';
  signal w34_901 : coaxsig := '0';
  signal w34_902 : coaxsig := '0';
  signal w34_903 : coaxsig := '0';
  signal w34_904 : coaxsig := '0';
  signal w34_905 : coaxsig := '0';
  signal w34_906 : coaxsig := '0';
  signal w34_907 : coaxsig := '0';
  signal w34_91 : coaxsig := '0';
  signal w34_92 : coaxsig := '0';
  signal w34_93 : coaxsig := '0';
  signal w34_94 : coaxsig := '0';
  signal w34_95 : coaxsig := '0';
  signal w34_96 : coaxsig := '0';
  signal w34_97 : coaxsig := '0';
  signal w34_98 : coaxsig := '0';
  signal w34_99 : coaxsig := '0';
  signal w35_90 : coaxsig := '0';
  signal w35_900 : coaxsig := '0';
  signal w35_901 : coaxsig := '0';
  signal w35_902 : coaxsig := '0';
  signal w35_903 : coaxsig := '0';
  signal w35_904 : coaxsig := '0';
  signal w35_905 : coaxsig := '0';
  signal w35_906 : coaxsig := '0';
  signal w35_907 : coaxsig := '0';
  signal w35_91 : coaxsig := '0';
  signal w35_92 : coaxsig := '0';
  signal w35_93 : coaxsig := '0';
  signal w35_94 : coaxsig := '0';
  signal w35_95 : coaxsig := '0';
  signal w35_96 : coaxsig := '0';
  signal w35_97 : coaxsig := '0';
  signal w35_98 : coaxsig := '0';
  signal w35_99 : coaxsig := '0';
  signal w36_90 : coaxsig := '0';
  signal w36_900 : coaxsig := '0';
  signal w36_901 : coaxsig := '0';
  signal w36_902 : coaxsig := '0';
  signal w36_903 : coaxsig := '0';
  signal w36_904 : coaxsig := '0';
  signal w36_905 : coaxsig := '0';
  signal w36_906 : coaxsig := '0';
  signal w36_907 : coaxsig := '0';
  signal w36_91 : coaxsig := '0';
  signal w36_92 : coaxsig := '0';
  signal w36_93 : coaxsig := '0';
  signal w36_94 : coaxsig := '0';
  signal w36_95 : coaxsig := '0';
  signal w36_96 : coaxsig := '0';
  signal w36_97 : coaxsig := '0';
  signal w36_98 : coaxsig := '0';
  signal w36_99 : coaxsig := '0';
  signal w999_90 : std_logic := '0';
  signal w999_91 : std_logic := '0';
  signal w999_92 : std_logic := '0';
begin
   --  Component instantiation.
   uut: cdc6600 port map (clk1 => clk1,
                          clk2 => clk2,
                          clk3 => clk3,
                          clk4 => clk4,
                          w02_90 => w02_90,
                          w02_900 => w02_900,
                          w02_901 => w02_901,
                          w02_902 => w02_902,
                          w02_903 => w02_903,
                          w02_904 => w02_904,
                          w02_905 => w02_905,
                          w02_906 => w02_906,
                          w02_907 => w02_907,
                          w02_91 => w02_91,
                          w02_92 => w02_92,
                          w02_93 => w02_93,
                          w02_94 => w02_94,
                          w02_95 => w02_95,
                          w02_96 => w02_96,
                          w02_97 => w02_97,
                          w02_98 => w02_98,
                          w02_99 => w02_99,
                          w03_900 => w03_900,
                          w03_901 => w03_901,
                          w03_902 => w03_902,
                          w03_903 => w03_903,
                          w03_904 => w03_904,
                          w03_905 => w03_905,
                          w03_906 => w03_906,
                          w03_907 => w03_907,
                          w03_908 => w03_908,
                          w03_91 => w03_91,
                          w03_92 => w03_92,
                          w03_93 => w03_93,
                          w03_94 => w03_94,
                          w03_95 => w03_95,
                          w03_96 => w03_96,
                          w03_97 => w03_97,
                          w03_98 => w03_98,
                          w03_99 => w03_99,
                          w04_90 => w04_90,
                          w04_900 => w04_900,
                          w04_901 => w04_901,
                          w04_902 => w04_902,
                          w04_903 => w04_903,
                          w04_904 => w04_904,
                          w04_905 => w04_905,
                          w04_91 => w04_91,
                          w04_92 => w04_92,
                          w04_93 => w04_93,
                          w04_94 => w04_94,
                          w04_95 => w04_95,
                          w04_96 => w04_96,
                          w04_97 => w04_97,
                          w04_98 => w04_98,
                          w04_99 => w04_99,
                          w05_90 => w05_90,
                          w05_900 => w05_900,
                          w05_901 => w05_901,
                          w05_902 => w05_902,
                          w05_903 => w05_903,
                          w05_904 => w05_904,
                          w05_905 => w05_905,
                          w05_906 => w05_906,
                          w05_907 => w05_907,
                          w05_91 => w05_91,
                          w05_92 => w05_92,
                          w05_93 => w05_93,
                          w05_94 => w05_94,
                          w05_95 => w05_95,
                          w05_96 => w05_96,
                          w05_97 => w05_97,
                          w05_98 => w05_98,
                          w05_99 => w05_99,
                          w06_905 => w06_905,
                          w06_90 => w06_90,
                          w06_900 => w06_900,
                          w06_901 => w06_901,
                          w06_902 => w06_902,
                          w06_903 => w06_903,
                          w06_904 => w06_904,
                          w06_906 => w06_906,
                          w06_907 => w06_907,
                          w06_91 => w06_91,
                          w06_92 => w06_92,
                          w06_93 => w06_93,
                          w06_94 => w06_94,
                          w06_95 => w06_95,
                          w06_96 => w06_96,
                          w06_97 => w06_97,
                          w07_90 => w07_90,
                          w07_900 => w07_900,
                          w07_901 => w07_901,
                          w07_902 => w07_902,
                          w07_903 => w07_903,
                          w07_904 => w07_904,
                          w07_905 => w07_905,
                          w07_906 => w07_906,
                          w07_907 => w07_907,
                          w07_91 => w07_91,
                          w07_92 => w07_92,
                          w07_93 => w07_93,
                          w07_94 => w07_94,
                          w07_95 => w07_95,
                          w07_96 => w07_96,
                          w07_97 => w07_97,
                          w08_90 => w08_90,
                          w08_900 => w08_900,
                          w08_901 => w08_901,
                          w08_902 => w08_902,
                          w08_903 => w08_903,
                          w08_904 => w08_904,
                          w08_905 => w08_905,
                          w08_91 => w08_91,
                          w08_92 => w08_92,
                          w08_93 => w08_93,
                          w08_94 => w08_94,
                          w08_95 => w08_95,
                          w08_96 => w08_96,
                          w08_97 => w08_97,
                          w08_98 => w08_98,
                          w08_99 => w08_99,
                          w09_90 => w09_90,
                          w09_900 => w09_900,
                          w09_901 => w09_901,
                          w09_902 => w09_902,
                          w09_903 => w09_903,
                          w09_904 => w09_904,
                          w09_91 => w09_91,
                          w09_92 => w09_92,
                          w09_93 => w09_93,
                          w09_94 => w09_94,
                          w09_95 => w09_95,
                          w09_96 => w09_96,
                          w09_97 => w09_97,
                          w09_98 => w09_98,
                          w09_99 => w09_99,
                          w10_90 => w10_90,
                          w10_900 => w10_900,
                          w10_901 => w10_901,
                          w10_902 => w10_902,
                          w10_903 => w10_903,
                          w10_904 => w10_904,
                          w10_91 => w10_91,
                          w10_92 => w10_92,
                          w10_93 => w10_93,
                          w10_94 => w10_94,
                          w10_95 => w10_95,
                          w10_96 => w10_96,
                          w10_97 => w10_97,
                          w10_98 => w10_98,
                          w10_99 => w10_99,
                          w11_90 => w11_90,
                          w11_900 => w11_900,
                          w11_901 => w11_901,
                          w11_902 => w11_902,
                          w11_903 => w11_903,
                          w11_904 => w11_904,
                          w11_91 => w11_91,
                          w11_92 => w11_92,
                          w11_93 => w11_93,
                          w11_94 => w11_94,
                          w11_95 => w11_95,
                          w11_96 => w11_96,
                          w11_97 => w11_97,
                          w11_98 => w11_98,
                          w11_99 => w11_99,
                          w12_900 => w12_900,
                          w12_901 => w12_901,
                          w12_902 => w12_902,
                          w12_903 => w12_903,
                          w12_904 => w12_904,
                          w12_905 => w12_905,
                          -- w13_90 => w13_90,
                          -- w13_900 => w13_900,
                          -- w13_901 => w13_901,
                          -- w13_902 => w13_902,
                          -- w13_903 => w13_903,
                          -- w13_904 => w13_904,
                          -- w13_905 => w13_905,
                          -- w13_906 => w13_906,
                          -- w13_907 => w13_907,
                          -- w13_91 => w13_91,
                          -- w13_92 => w13_92,
                          -- w13_93 => w13_93,
                          -- w13_94 => w13_94,
                          -- w13_95 => w13_95,
                          -- w13_96 => w13_96,
                          -- w13_97 => w13_97,
                          -- w13_98 => w13_98,
                          -- w13_99 => w13_99,
                          -- w14_90 => w14_90,
                          -- w14_900 => w14_900,
                          -- w14_901 => w14_901,
                          -- w14_902 => w14_902,
                          -- w14_903 => w14_903,
                          -- w14_904 => w14_904,
                          -- w14_905 => w14_905,
                          -- w14_906 => w14_906,
                          -- w14_907 => w14_907,
                          -- w14_91 => w14_91,
                          -- w14_92 => w14_92,
                          -- w14_93 => w14_93,
                          -- w14_94 => w14_94,
                          -- w14_95 => w14_95,
                          -- w14_96 => w14_96,
                          -- w14_97 => w14_97,
                          -- w14_98 => w14_98,
                          -- w14_99 => w14_99,
                          w15_90 => w15_90,
                          w15_900 => w15_900,
                          w15_901 => w15_901,
                          w15_902 => w15_902,
                          w15_903 => w15_903,
                          w15_904 => w15_904,
                          w15_905 => w15_905,
                          w15_906 => w15_906,
                          w15_907 => w15_907,
                          w15_91 => w15_91,
                          w15_92 => w15_92,
                          w15_93 => w15_93,
                          w15_94 => w15_94,
                          w15_95 => w15_95,
                          w15_96 => w15_96,
                          w15_97 => w15_97,
                          w15_98 => w15_98,
                          w15_99 => w15_99,
                          w16_90 => w16_90,
                          w16_900 => w16_900,
                          w16_901 => w16_901,
                          w16_902 => w16_902,
                          w16_903 => w16_903,
                          w16_904 => w16_904,
                          w16_905 => w16_905,
                          w16_906 => w16_906,
                          w16_907 => w16_907,
                          w16_91 => w16_91,
                          w16_92 => w16_92,
                          w16_93 => w16_93,
                          w16_94 => w16_94,
                          w16_95 => w16_95,
                          w16_96 => w16_96,
                          w16_97 => w16_97,
                          w16_98 => w16_98,
                          w16_99 => w16_99,
                          w17_90 => w17_90,
                          w17_900 => w17_900,
                          w17_901 => w17_901,
                          w17_902 => w17_902,
                          w17_903 => w17_903,
                          w17_904 => w17_904,
                          w17_905 => w17_905,
                          w17_906 => w17_906,
                          w17_907 => w17_907,
                          w17_91 => w17_91,
                          w17_92 => w17_92,
                          w17_93 => w17_93,
                          w17_94 => w17_94,
                          w17_95 => w17_95,
                          w17_96 => w17_96,
                          w17_97 => w17_97,
                          w17_98 => w17_98,
                          w17_99 => w17_99,
                          w18_90 => w18_90,
                          w18_900 => w18_900,
                          w18_901 => w18_901,
                          w18_902 => w18_902,
                          w18_903 => w18_903,
                          w18_904 => w18_904,
                          w18_905 => w18_905,
                          w18_906 => w18_906,
                          w18_907 => w18_907,
                          w18_91 => w18_91,
                          w18_92 => w18_92,
                          w18_93 => w18_93,
                          w18_94 => w18_94,
                          w18_95 => w18_95,
                          w18_96 => w18_96,
                          w18_97 => w18_97,
                          w18_98 => w18_98,
                          w18_99 => w18_99,
                          w19_90 => w19_90,
                          w19_900 => w19_900,
                          w19_901 => w19_901,
                          w19_902 => w19_902,
                          w19_903 => w19_903,
                          w19_904 => w19_904,
                          w19_905 => w19_905,
                          w19_906 => w19_906,
                          w19_907 => w19_907,
                          w19_91 => w19_91,
                          w19_92 => w19_92,
                          w19_93 => w19_93,
                          w19_94 => w19_94,
                          w19_95 => w19_95,
                          w19_96 => w19_96,
                          w19_97 => w19_97,
                          w19_98 => w19_98,
                          w19_99 => w19_99,
                          w20_90 => w20_90,
                          w20_900 => w20_900,
                          w20_901 => w20_901,
                          w20_902 => w20_902,
                          w20_903 => w20_903,
                          w20_904 => w20_904,
                          w20_905 => w20_905,
                          w20_906 => w20_906,
                          w20_907 => w20_907,
                          w20_91 => w20_91,
                          w20_92 => w20_92,
                          w20_93 => w20_93,
                          w20_94 => w20_94,
                          w20_95 => w20_95,
                          w20_96 => w20_96,
                          w20_97 => w20_97,
                          w20_98 => w20_98,
                          w20_99 => w20_99,
                          w21_90 => w21_90,
                          w21_900 => w21_900,
                          w21_901 => w21_901,
                          w21_902 => w21_902,
                          w21_903 => w21_903,
                          w21_904 => w21_904,
                          w21_905 => w21_905,
                          w21_906 => w21_906,
                          w21_907 => w21_907,
                          w21_91 => w21_91,
                          w21_92 => w21_92,
                          w21_93 => w21_93,
                          w21_94 => w21_94,
                          w21_95 => w21_95,
                          w21_96 => w21_96,
                          w21_97 => w21_97,
                          w21_98 => w21_98,
                          w21_99 => w21_99,
                          w22_90 => w22_90,
                          w22_900 => w22_900,
                          w22_901 => w22_901,
                          w22_902 => w22_902,
                          w22_903 => w22_903,
                          w22_904 => w22_904,
                          w22_905 => w22_905,
                          w22_906 => w22_906,
                          w22_907 => w22_907,
                          w22_91 => w22_91,
                          w22_92 => w22_92,
                          w22_93 => w22_93,
                          w22_94 => w22_94,
                          w22_95 => w22_95,
                          w22_96 => w22_96,
                          w22_97 => w22_97,
                          w22_98 => w22_98,
                          w22_99 => w22_99,
                          w23_90 => w23_90,
                          w23_900 => w23_900,
                          w23_901 => w23_901,
                          w23_902 => w23_902,
                          w23_903 => w23_903,
                          w23_904 => w23_904,
                          w23_905 => w23_905,
                          w23_906 => w23_906,
                          w23_907 => w23_907,
                          w23_91 => w23_91,
                          w23_92 => w23_92,
                          w23_93 => w23_93,
                          w23_94 => w23_94,
                          w23_95 => w23_95,
                          w23_96 => w23_96,
                          w23_97 => w23_97,
                          w23_98 => w23_98,
                          w23_99 => w23_99,
                          w24_90 => w24_90,
                          w24_900 => w24_900,
                          w24_901 => w24_901,
                          w24_902 => w24_902,
                          w24_903 => w24_903,
                          w24_904 => w24_904,
                          w24_905 => w24_905,
                          w24_906 => w24_906,
                          w24_907 => w24_907,
                          w24_91 => w24_91,
                          w24_92 => w24_92,
                          w24_93 => w24_93,
                          w24_94 => w24_94,
                          w24_95 => w24_95,
                          w24_96 => w24_96,
                          w24_97 => w24_97,
                          w24_98 => w24_98,
                          w24_99 => w24_99,
                          w25_90 => w25_90,
                          w25_900 => w25_900,
                          w25_901 => w25_901,
                          w25_902 => w25_902,
                          w25_903 => w25_903,
                          w25_904 => w25_904,
                          w25_905 => w25_905,
                          w25_906 => w25_906,
                          w25_907 => w25_907,
                          w25_91 => w25_91,
                          w25_92 => w25_92,
                          w25_93 => w25_93,
                          w25_94 => w25_94,
                          w25_95 => w25_95,
                          w25_96 => w25_96,
                          w25_97 => w25_97,
                          w25_98 => w25_98,
                          w25_99 => w25_99,
                          w26_90 => w26_90,
                          w26_900 => w26_900,
                          w26_901 => w26_901,
                          w26_902 => w26_902,
                          w26_903 => w26_903,
                          w26_904 => w26_904,
                          w26_905 => w26_905,
                          w26_906 => w26_906,
                          w26_907 => w26_907,
                          w26_91 => w26_91,
                          w26_92 => w26_92,
                          w26_93 => w26_93,
                          w26_94 => w26_94,
                          w26_95 => w26_95,
                          w26_96 => w26_96,
                          w26_97 => w26_97,
                          w26_98 => w26_98,
                          w26_99 => w26_99,
                          w27_90 => w27_90,
                          w27_900 => w27_900,
                          w27_901 => w27_901,
                          w27_902 => w27_902,
                          w27_903 => w27_903,
                          w27_904 => w27_904,
                          w27_905 => w27_905,
                          w27_906 => w27_906,
                          w27_907 => w27_907,
                          w27_91 => w27_91,
                          w27_92 => w27_92,
                          w27_93 => w27_93,
                          w27_94 => w27_94,
                          w27_95 => w27_95,
                          w27_96 => w27_96,
                          w27_97 => w27_97,
                          w27_98 => w27_98,
                          w27_99 => w27_99,
                          w28_90 => w28_90,
                          w28_900 => w28_900,
                          w28_901 => w28_901,
                          w28_902 => w28_902,
                          w28_903 => w28_903,
                          w28_904 => w28_904,
                          w28_905 => w28_905,
                          w28_906 => w28_906,
                          w28_907 => w28_907,
                          w28_91 => w28_91,
                          w28_92 => w28_92,
                          w28_93 => w28_93,
                          w28_94 => w28_94,
                          w28_95 => w28_95,
                          w28_96 => w28_96,
                          w28_97 => w28_97,
                          w28_98 => w28_98,
                          w28_99 => w28_99,
                          w31_90 => w31_90,
                          w31_900 => w31_900,
                          w31_901 => w31_901,
                          w31_902 => w31_902,
                          w31_903 => w31_903,
                          w31_904 => w31_904,
                          w31_905 => w31_905,
                          w31_906 => w31_906,
                          w31_907 => w31_907,
                          w31_91 => w31_91,
                          w31_92 => w31_92,
                          w31_93 => w31_93,
                          w31_94 => w31_94,
                          w31_95 => w31_95,
                          w31_96 => w31_96,
                          w31_97 => w31_97,
                          w31_98 => w31_98,
                          w31_99 => w31_99,
                          w32_90 => w32_90,
                          w32_900 => w32_900,
                          w32_901 => w32_901,
                          w32_902 => w32_902,
                          w32_903 => w32_903,
                          w32_904 => w32_904,
                          w32_905 => w32_905,
                          w32_906 => w32_906,
                          w32_907 => w32_907,
                          w32_91 => w32_91,
                          w32_92 => w32_92,
                          w32_93 => w32_93,
                          w32_94 => w32_94,
                          w32_95 => w32_95,
                          w32_96 => w32_96,
                          w32_97 => w32_97,
                          w32_98 => w32_98,
                          w32_99 => w32_99,
                          w33_90 => w33_90,
                          w33_900 => w33_900,
                          w33_901 => w33_901,
                          w33_902 => w33_902,
                          w33_903 => w33_903,
                          w33_904 => w33_904,
                          w33_905 => w33_905,
                          w33_906 => w33_906,
                          w33_907 => w33_907,
                          w33_91 => w33_91,
                          w33_92 => w33_92,
                          w33_93 => w33_93,
                          w33_94 => w33_94,
                          w33_95 => w33_95,
                          w33_96 => w33_96,
                          w33_97 => w33_97,
                          w33_98 => w33_98,
                          w33_99 => w33_99,
                          w34_90 => w34_90,
                          w34_900 => w34_900,
                          w34_901 => w34_901,
                          w34_902 => w34_902,
                          w34_903 => w34_903,
                          w34_904 => w34_904,
                          w34_905 => w34_905,
                          w34_906 => w34_906,
                          w34_907 => w34_907,
                          w34_91 => w34_91,
                          w34_92 => w34_92,
                          w34_93 => w34_93,
                          w34_94 => w34_94,
                          w34_95 => w34_95,
                          w34_96 => w34_96,
                          w34_97 => w34_97,
                          w34_98 => w34_98,
                          w34_99 => w34_99,
                          w35_90 => w35_90,
                          w35_900 => w35_900,
                          w35_901 => w35_901,
                          w35_902 => w35_902,
                          w35_903 => w35_903,
                          w35_904 => w35_904,
                          w35_905 => w35_905,
                          w35_906 => w35_906,
                          w35_907 => w35_907,
                          w35_91 => w35_91,
                          w35_92 => w35_92,
                          w35_93 => w35_93,
                          w35_94 => w35_94,
                          w35_95 => w35_95,
                          w35_96 => w35_96,
                          w35_97 => w35_97,
                          w35_98 => w35_98,
                          w35_99 => w35_99,
                          w36_90 => w36_90,
                          w36_900 => w36_900,
                          w36_901 => w36_901,
                          w36_902 => w36_902,
                          w36_903 => w36_903,
                          w36_904 => w36_904,
                          w36_905 => w36_905,
                          w36_906 => w36_906,
                          w36_907 => w36_907,
                          w36_91 => w36_91,
                          w36_92 => w36_92,
                          w36_93 => w36_93,
                          w36_94 => w36_94,
                          w36_95 => w36_95,
                          w36_96 => w36_96,
                          w36_97 => w36_97,
                          w36_98 => w36_98,
                          w36_99 => w36_99,
                          w999_90 => w999_90,
                          w999_91 => w999_91,
                          w999_92 => w999_92);
   --  This process does the real job.
   -- purpose: Read the test script and pass it to the UUT
   -- type   : combinational
   -- inputs : 
   -- outputs: 
   test: process
     constant idle : coaxsigs := ('0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0');
     variable testdata : testvec; -- One line worth of test data
     variable l : line;
     file vector_file : text is in "./cdc_tb.txt";  -- test vector file
     variable g : boolean;
     variable b : character;
     variable i : integer;
     variable d : integer;
     variable ic, oc : coaxsigs;
     variable c1, c2, c3, c4 : integer := 0;
     variable x, y : integer := 0;
     variable unblank : integer := 0;
     constant space : string := " ";
     constant zero : string := "0";
     constant one : string := "1";
     variable llen : integer;
   begin  -- process test
     dtmain;
     while not endfile (vector_file) loop
       readline (vector_file, l);
       read (l, d);                     -- delay in 25 ns units
       read (l, b);                     -- skip the space separator
       for i in testdata'left to testdata'right loop
         read (l, b, good => g);
         exit when not g;
         llen := i;
         if b = '0' then
           testdata(i) := '0';
         elsif b = '1' then
           testdata(i) := '1';
         end if;
       end loop;  -- i
       for i in 1 to 19 loop
         -- coax1(i) <= testdata(i);
         oc(i) := testdata(i);
       end loop;  -- i
       write(l, d);
       write (l, space);
       for i in testdata'left to llen loop
         if testdata(i) = '0' then
           write (l, zero);
         else
           write (l, one);
         end if;
       end loop;  -- i
       writeline (output, l);
       w999_90 <= testdata (1);
       w999_91 <= testdata (2);
       w999_92 <= testdata (3);
       for i in 1 to d loop
         wait for 25 ns;
         if clk1 = '1' then
           clk1 <= '0';
           clk2 <= '1';
         elsif clk2 = '1' then
           clk2 <= '0';
           clk3 <= '1';
         elsif clk3 = '1' then
           clk3 <= '0';
           clk4 <= '1';
         elsif clk4 = '1' then
           clk4 <= '0';
           clk1 <= '1';
         end if;
         oc := idle;
       end loop;  -- i
     end loop;
     assert false report "end of test";
     --  Wait forever; this will finish the simulation.
     wait;
   end process test;
end;
