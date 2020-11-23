"""
This script calls takeData.takeData() in a loop. 
28 July 2016 -- added try/except after segmentation violation overnight. 
"""

import time
import takeData

# take struck data in a loop!
takeData.takeData(doLoop=True,n_hours=20.0) # set up the digitizer, take data

