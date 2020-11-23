#include "NGMParticleIdent.h"
#include "NGMSystemConfiguration.h"
#include "NGMHit.h"
#include "NGMLogger.h"
#include "TColor.h"
#include "TLegend.h"
#include "TMarker.h"
#include <math.h>

ClassImp(partSelect_rst);

partSelect_rst::partSelect_rst()
{
  partid = -1;
  energymin = -1;
  energymax = -1;
}

