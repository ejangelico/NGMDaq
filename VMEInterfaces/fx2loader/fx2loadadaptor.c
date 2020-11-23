/***************************************************************************/
/*  Filename:  usrpadaptor.cpp                                             */
/*                                                                         */
/*  Funktion:   We rely on some functions from the linux packet            */
/*              radio project (usrp) to load the fx2 firmware.  These      */
/*              functions were written to be called in C++ and our lib     */
/*              is suposed to be C.  This file is a little adaptor between */
/*              the two                                                    */
/*                                                                         */
/*  Autor:      R. Fox                                                     */
/* date:        18.05.2006  (coding begins)                                */
/*-------------------------------------------------------------------------*/
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2006                                                                 */
/*                                                                         */
/***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "argtable2.h"
#include "libusbwrap.h"

#include <usb.h>
#include "libfx2loader.h"
#include "liberror.h"
#include "libbuffer.h"

int 
fx2_load_firmware( const char* filename)
{
  
  const char *error = NULL;
  struct Buffer sourceData = {0};
  struct Buffer sourceMask = {0};
  int returnCode = 0;
  struct usb_dev_handle* device = NULL;

  if ( usbOpenDeviceVP("1657:3150", 1, 0, 0, &device, &error) ) {
    fprintf(stderr, "%s\n", error);
    FAIL(5);
  }

  // Initialise buffers...
  //
  if ( bufInitialise(&sourceData, 1024, 0x00, &error) ) {
    fprintf(stderr, "%s\n", error);
    FAIL(1);
  }
  if ( bufInitialise(&sourceMask, 1024, 0x00, &error) ) {
      fprintf(stderr, "%s\n", error);
      FAIL(2);
  }
  if ( bufReadFromIntelHexFile(&sourceData, &sourceMask, filename, &error) ) {
      fprintf(stderr, "%s\n", error);
      FAIL(3);
  }
      
  // Write the data to RAM
  //
  if ( fx2WriteRAM(device, sourceData.data, sourceData.length, &error) ) {
    fprintf(stderr, "%s\n", error);
    FAIL(5);
  }

  cleanup:
  if ( device ) {
    usb_release_interface(device, 0);
    usb_close(device);
  }
  
  if ( error ) {
    fx2FreeError(error);
  }
  if ( sourceMask.data ) {
    bufDestroy(&sourceMask);
  }
  if ( sourceData.data ) {
    bufDestroy(&sourceData);
  }
  return returnCode;
}
