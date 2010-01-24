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
      p2 : out logicsig;
      p3 : out logicsig;
      p4 : out logicsig;
      p5 : out logicsig;
      p6 : out logicsig);

end za;
architecture gates of za is


begin -- gates

  p2 <= p1;
  p3 <= p1;
  p4 <= p1;
  p5 <= p1;
  p6 <= p1;

end gates;

