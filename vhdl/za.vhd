-------------------------------------------------------------------------------
--
-- CDC 6600 model
--
-- Copyright (C) 2010 by Paul Koning
--
-- Z series "modules" do things that aren't in the original design.
--
-- ZA module
--
-------------------------------------------------------------------------------

use work.sigs.all;

entity za is
    port (
      p1 : in  logicsig;
      p2_p3_p4_p5_p6 : out logicsig);

end za;
architecture gates of za is


begin -- gates

  p2_p3_p4_p5_p6 <= p1;

end gates;

