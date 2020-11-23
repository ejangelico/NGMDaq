/* $Id$ */
/* Author:  Peter Barnes 2008-12 */

/* COMMENTS LIKE THIS ONLY    NO '--' COMMENTS */

/* View of countdist ntuple joined to carwash runs */

CREATE OR REPLACE 
  ALGORITHM=UNDEFINED
  DEFINER=ngmdaq@'%' 
  SQL SECURITY DEFINER 
  
  VIEW ngmrunlog.<table> AS 
    SELECT 
      IFNULL(LEFT(fl.experimentname,100),"") AS experimentname,
      IFNULL(LEFT(fl.testobjectid,  100),"") AS testobjectid,
      
      IFNULL(LEFT(cd.runnumber,     100),"") AS runnumber,
      IFNULL(LEFT(cd.pass,           20),"") AS pass,
      IFNULL(LEFT(cd.modulename,     20),"") AS modulename,

      IFNULL(cd.runduration, 0) AS runduration,
      IFNULL(cd.r1,          0) AS r1,
      IFNULL(cd.r2f,         0) AS r2f,
      IFNULL(cd.lambda,      0) AS lambda,
      IFNULL(cd.r3f,         0) AS r3f,
      IFNULL(cd.r2fe,        0) AS r2fe,
      IFNULL(cd.lambdae,     0) AS lambdae,
      IFNULL(cd.r3fe,        0) AS r3fe,
      IFNULL(cd.y2fitstatus, 0) AS y2fitstatus,
      IFNULL(cd.y3fitstatus, 0) AS y3fitstatus,
      IFNULL(LEFT(cd.passid,       100), "") AS passid,
      
      IFNULL(cw.vFuel,       0) AS fuel,
      IFNULL(cw.vWeight,     0) AS weight,
      IFNULL(cw.vOccupants, -1) AS occupants
      
      FROM filelog              AS fl
        JOIN countdistlasttime  AS cd 
        ON fl.runnumber = cd.runnumber

        JOIN carwash            AS cw
        ON fl.experimentname = cw.runtag
 