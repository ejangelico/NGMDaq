/***************************************************************************/
/*  Filename:  usrpadaptor.h                                               */
/*                                                                         */
/*  Funktion:   We rely on some functions from the linux packet            */
/*              radio project (usrp) to load the fx2 firmware.  These      */
/*              functions were written to be called in C++ and our lib     */
/*              is suposed to be C.  This file is a header for a little    */
/*               adaptor between the two                                   */
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
/*  © 2006                                                                 */
/*                                                                         */
/***************************************************************************/

#ifndef __FX2LOADADAPTOR_H
#define __FX2LOADADAPTOR_H

#include <usb.h>

#ifdef __cplusplus
extern "C" {
#endif

int fx2adaptor_load_firmware(const char* filename);

#ifdef __cplusplus
}
#endif

#endif
