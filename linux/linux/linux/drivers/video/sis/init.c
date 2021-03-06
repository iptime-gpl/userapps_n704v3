/* $XFree86: xc/programs/Xserver/hw/xfree86/drivers/sis/init.c,v 1.3 2002/24/04 01:16:16 dawes Exp $ */
/*
 * Mode switching code (CRT1 section) for SiS 300/540/630/730/315/550/650/740
 * (Universal module for Linux kernel framebuffer and XFree86 4.x)
 *
 * Assembler-To-C translation
 * Parts Copyright 2002 by Thomas Winischhofer <thomas@winischhofer.net>
 *
 * Based on BIOS
 *     1.10.07 (1.10a) for SiS650/LVDS+CH7019
 *     1.07.1b for SiS650/301(B/LV)
 *     2.04.50 (I) and 2.04.5c (II), 2.07a for SiS630/301(B)
 *     2.02.3b, 2.03.02 and 2.04.5c for 630/LVDS/LVDS+CH7005
 *     1.09b for 315/301(B)
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of the copyright holder not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  The copyright holder makes no representations
 * about the suitability of this software for any purpose.  It is provided
 * "as is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDER DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 */

#include "init.h"

#ifdef SIS300
#include "300vtbl.h"
#endif

#ifdef SIS315H
#include "310vtbl.h"
#endif

#ifdef LINUX_XF86
BOOLEAN SiSBIOSSetMode(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,
                       ScrnInfoPtr pScrn, DisplayModePtr mode);
#ifdef SISDUALHEAD     /* TW: For dual head */
BOOLEAN SiSBIOSSetModeCRT1(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,
                       ScrnInfoPtr pScrn, DisplayModePtr mode);
BOOLEAN SiSBIOSSetModeCRT2(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,
                       ScrnInfoPtr pScrn, DisplayModePtr mode);
#endif /* dual head */
#endif /* linux_xf86 */

#ifdef LINUXBIOS
BOOLEAN SiSInit(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension);
#endif

#ifdef LINUX_XF86
BOOLEAN SiSSetMode(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,
                   ScrnInfoPtr pScrn,USHORT ModeNo, BOOLEAN dosetpitch);
#else
BOOLEAN SiSSetMode(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,
                   USHORT ModeNo);
#endif

#if defined(ALLOC_PRAGMA)
#pragma alloc_text(PAGE,SiSSetMode)
#pragma alloc_text(PAGE,SiSInit)
#endif

void DelaySeconds(int seconds);
void DebugCode(SiS_Private *SiS_Pr, UCHAR code);

#ifdef LINUX_XF86
/* TW: Mode table for X driver */
const UShort  ModeIndex_320x480[]      = {0x5A, 0x5B, 0x00, 0x00};  /* DSTN/FSTN */
const UShort  ModeIndex_512x384[]      = {0x52, 0x58, 0x00, 0x5c};
const UShort  ModeIndex_640x480[]      = {0x2E, 0x44, 0x00, 0x62};
const UShort  ModeIndex_720x480[]      = {0x31, 0x33, 0x00, 0x35};
const UShort  ModeIndex_720x576[]      = {0x32, 0x34, 0x00, 0x36};
const UShort  ModeIndex_800x480[]      = {0x70, 0x7a, 0x00, 0x76};  /* 310/325 series only */
const UShort  ModeIndex_800x600[]      = {0x30, 0x47, 0x00, 0x63};
const UShort  ModeIndex_1024x768[]     = {0x38, 0x4A, 0x00, 0x64};
const UShort  ModeIndex_1024x576[]     = {0x71, 0x74, 0x00, 0x77};  /* 310/325 series only */
const UShort  ModeIndex_1024x600[]     = {0x20, 0x21, 0x00, 0x22};  /* 300 series only */
const UShort  ModeIndex_1280x1024[]    = {0x3A, 0x4D, 0x00, 0x65};
const UShort  ModeIndex_300_1280x960[] = {0x6e, 0x6f, 0x00, 0x7b};
const UShort  ModeIndex_310_1280x960[] = {0x7C, 0x7D, 0x00, 0x7E};
const UShort  ModeIndex_1152x768[]     = {0x23, 0x24, 0x00, 0x25};  /* 300 series only */
const UShort  ModeIndex_1280x768[]     = {0x23, 0x24, 0x00, 0x25};  /* 310/325 series only */
const UShort  ModeIndex_1280x720[]     = {0x79, 0x75, 0x00, 0x78};  /* 310/325 series only */
const UShort  ModeIndex_1400x1050[]    = {0x26, 0x27, 0x00, 0x28};  /* 310/325 series only */
const UShort  ModeIndex_1600x1200[]    = {0x3C, 0x3D, 0x00, 0x66};
const UShort  ModeIndex_1920x1440[]    = {0x68, 0x69, 0x00, 0x6B};
const UShort  ModeIndex_2048x1536[]    = {0x6c, 0x6d, 0x00, 0x6e};  /* 310/325 series only */
#endif

void
DelaySeconds(int seconds)
{
  int i;
#ifdef WIN2000
  int j;
#endif

  for (i=0;i<seconds;i++) {
#ifdef TC
    delay(1000);
#endif

#ifdef WIN2000
    for (j=0;j<20000;j++)
      VideoPortStallExecution(50);
#endif

#ifdef WINCE_HEADER
#endif

#ifdef LINUX_KERNEL
#endif
  }
}

void
DebugCode(SiS_Private *SiS_Pr, UCHAR code)
{
  OutPortByte(0x80, code);
  DelaySeconds(0x3);
}

#ifdef SIS300
void
InitTo300Pointer(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   SiS_Pr->SiS_SModeIDTable  = (SiS_StStruct *)SiS300_SModeIDTable;
   SiS_Pr->SiS_VBModeIDTable = (SiS_VBModeStruct *)SiS300_VBModeIDTable;
   SiS_Pr->SiS_StandTable    = (SiS_StandTableStruct *)SiS300_StandTable;
   SiS_Pr->SiS_EModeIDTable  = (SiS_ExtStruct *)SiS300_EModeIDTable;
   SiS_Pr->SiS_RefIndex      = (SiS_Ext2Struct *)SiS300_RefIndex;
   SiS_Pr->SiS_CRT1Table     = (SiS_CRT1TableStruct *)SiS300_CRT1Table;
   if(HwDeviceExtension->jChipType == SIS_300) {
      SiS_Pr->SiS_MCLKData_0    = (SiS_MCLKDataStruct *)SiS300_MCLKData_300; /* 300 */
   } else {
      SiS_Pr->SiS_MCLKData_0    = (SiS_MCLKDataStruct *)SiS300_MCLKData_630; /* 630 */
   }
   SiS_Pr->SiS_ECLKData      = (SiS_ECLKDataStruct *)SiS300_ECLKData;
   SiS_Pr->SiS_VCLKData      = (SiS_VCLKDataStruct *)SiS300_VCLKData;
   SiS_Pr->SiS_VBVCLKData    = (SiS_VBVCLKDataStruct *)SiS300_VCLKData;
   SiS_Pr->SiS_ScreenOffset  = SiS300_ScreenOffset;
   SiS_Pr->SiS_StResInfo     = (SiS_StResInfoStruct *)SiS300_StResInfo;
   SiS_Pr->SiS_ModeResInfo   = (SiS_ModeResInfoStruct *)SiS300_ModeResInfo;

   SiS_Pr->pSiS_OutputSelect = &SiS300_OutputSelect;
   SiS_Pr->pSiS_SoftSetting  = &SiS300_SoftSetting;

   SiS_Pr->SiS_SR15  = SiS300_SR15;

#ifndef LINUX_XF86
   SiS_Pr->pSiS_SR07 = &SiS300_SR07;
   SiS_Pr->SiS_CR40  = SiS300_CR40;
   SiS_Pr->SiS_CR49  = SiS300_CR49;
   SiS_Pr->pSiS_SR1F = &SiS300_SR1F;
   SiS_Pr->pSiS_SR21 = &SiS300_SR21;
   SiS_Pr->pSiS_SR22 = &SiS300_SR22;
   SiS_Pr->pSiS_SR23 = &SiS300_SR23;
   SiS_Pr->pSiS_SR24 = &SiS300_SR24;
   SiS_Pr->SiS_SR25  = SiS300_SR25;
   SiS_Pr->pSiS_SR31 = &SiS300_SR31;
   SiS_Pr->pSiS_SR32 = &SiS300_SR32;
   SiS_Pr->pSiS_SR33 = &SiS300_SR33;
   SiS_Pr->pSiS_CRT2Data_1_2  = &SiS300_CRT2Data_1_2;
   SiS_Pr->pSiS_CRT2Data_4_D  = &SiS300_CRT2Data_4_D;
   SiS_Pr->pSiS_CRT2Data_4_E  = &SiS300_CRT2Data_4_E;
   SiS_Pr->pSiS_CRT2Data_4_10 = &SiS300_CRT2Data_4_10;
   SiS_Pr->pSiS_RGBSenseData    = &SiS300_RGBSenseData;
   SiS_Pr->pSiS_VideoSenseData  = &SiS300_VideoSenseData;
   SiS_Pr->pSiS_YCSenseData     = &SiS300_YCSenseData;
   SiS_Pr->pSiS_RGBSenseData2   = &SiS300_RGBSenseData2;
   SiS_Pr->pSiS_VideoSenseData2 = &SiS300_VideoSenseData2;
   SiS_Pr->pSiS_YCSenseData2    = &SiS300_YCSenseData2;
#endif

   SiS_Pr->SiS_NTSCPhase  = SiS300_NTSCPhase;
   SiS_Pr->SiS_PALPhase   = SiS300_PALPhase;
   SiS_Pr->SiS_NTSCPhase2 = SiS300_NTSCPhase2;
   SiS_Pr->SiS_PALPhase2  = SiS300_PALPhase2;
   SiS_Pr->SiS_PALMPhase  = SiS300_PALMPhase;
   SiS_Pr->SiS_PALNPhase  = SiS300_PALNPhase;
   SiS_Pr->SiS_PALMPhase2 = SiS300_PALMPhase2;
   SiS_Pr->SiS_PALNPhase2 = SiS300_PALNPhase2;

   SiS_Pr->SiS_StLCD1024x768Data    = (SiS_LCDDataStruct *)SiS300_StLCD1024x768Data;
   SiS_Pr->SiS_ExtLCD1024x768Data   = (SiS_LCDDataStruct *)SiS300_ExtLCD1024x768Data;
   SiS_Pr->SiS_St2LCD1024x768Data   = (SiS_LCDDataStruct *)SiS300_St2LCD1024x768Data;
   SiS_Pr->SiS_StLCD1280x1024Data   = (SiS_LCDDataStruct *)SiS300_StLCD1280x1024Data;
   SiS_Pr->SiS_ExtLCD1280x1024Data  = (SiS_LCDDataStruct *)SiS300_ExtLCD1280x1024Data;
   SiS_Pr->SiS_St2LCD1280x1024Data  = (SiS_LCDDataStruct *)SiS300_St2LCD1280x1024Data;
   SiS_Pr->SiS_NoScaleData1024x768  = (SiS_LCDDataStruct *)SiS300_NoScaleData1024x768;
   SiS_Pr->SiS_NoScaleData1280x1024 = (SiS_LCDDataStruct *)SiS300_NoScaleData1280x1024;
   SiS_Pr->SiS_LCD1280x960Data      = (SiS_LCDDataStruct *)SiS300_LCD1280x960Data;
   SiS_Pr->SiS_ExtLCD1400x1050Data  = (SiS_LCDDataStruct *)SiS300_ExtLCD1400x1050Data;
   SiS_Pr->SiS_ExtLCD1600x1200Data  = (SiS_LCDDataStruct *)SiS300_ExtLCD1600x1200Data;
   SiS_Pr->SiS_StLCD1400x1050Data   = (SiS_LCDDataStruct *)SiS300_StLCD1400x1050Data;
   SiS_Pr->SiS_StLCD1600x1200Data   = (SiS_LCDDataStruct *)SiS300_StLCD1600x1200Data;
   SiS_Pr->SiS_NoScaleData1400x1050 = (SiS_LCDDataStruct *)SiS300_NoScaleData1400x1050;
   SiS_Pr->SiS_NoScaleData1600x1200 = (SiS_LCDDataStruct *)SiS300_NoScaleData1600x1200;

   SiS_Pr->SiS_StPALData   = (SiS_TVDataStruct *)SiS300_StPALData;
   SiS_Pr->SiS_ExtPALData  = (SiS_TVDataStruct *)SiS300_ExtPALData;
   SiS_Pr->SiS_StNTSCData  = (SiS_TVDataStruct *)SiS300_StNTSCData;
   SiS_Pr->SiS_ExtNTSCData = (SiS_TVDataStruct *)SiS300_ExtNTSCData;
#ifdef oldHV
   SiS_Pr->SiS_St1HiTVData = (SiS_TVDataStruct *)SiS300_St1HiTVData;
   SiS_Pr->SiS_St2HiTVData = (SiS_TVDataStruct *)SiS300_St2HiTVData;
   SiS_Pr->SiS_ExtHiTVData = (SiS_TVDataStruct *)SiS300_ExtHiTVData;
#endif

   SiS_Pr->SiS_NTSCTiming     = SiS300_NTSCTiming;
   SiS_Pr->SiS_PALTiming      = SiS300_PALTiming;
#ifdef oldHV
   SiS_Pr->SiS_HiTVSt1Timing  = SiS300_HiTVSt1Timing;
   SiS_Pr->SiS_HiTVSt2Timing  = SiS300_HiTVSt2Timing;
   SiS_Pr->SiS_HiTVTextTiming = SiS300_HiTVTextTiming;
   SiS_Pr->SiS_HiTVGroup3Data = SiS300_HiTVGroup3Data;
   SiS_Pr->SiS_HiTVGroup3Simu = SiS300_HiTVGroup3Simu;
   SiS_Pr->SiS_HiTVGroup3Text = SiS300_HiTVGroup3Text;
#endif

   SiS_Pr->SiS_PanelDelayTbl     = (SiS_PanelDelayTblStruct *)SiS300_PanelDelayTbl;
   SiS_Pr->SiS_PanelDelayTblLVDS = (SiS_PanelDelayTblStruct *)SiS300_PanelDelayTblLVDS;

   SiS_Pr->SiS_LVDS800x600Data_1   = (SiS_LVDSDataStruct *)SiS300_LVDS800x600Data_1;
   SiS_Pr->SiS_LVDS800x600Data_2   = (SiS_LVDSDataStruct *)SiS300_LVDS800x600Data_2;
   SiS_Pr->SiS_LVDS1024x768Data_1  = (SiS_LVDSDataStruct *)SiS300_LVDS1024x768Data_1;
   SiS_Pr->SiS_LVDS1024x768Data_2  = (SiS_LVDSDataStruct *)SiS300_LVDS1024x768Data_2;
   SiS_Pr->SiS_LVDS1280x1024Data_1 = (SiS_LVDSDataStruct *)SiS300_LVDS1280x1024Data_1;
   SiS_Pr->SiS_LVDS1280x1024Data_2 = (SiS_LVDSDataStruct *)SiS300_LVDS1280x1024Data_2;
   SiS_Pr->SiS_LVDS1280x960Data_1  = (SiS_LVDSDataStruct *)SiS300_LVDS1280x1024Data_1;
   SiS_Pr->SiS_LVDS1280x960Data_2  = (SiS_LVDSDataStruct *)SiS300_LVDS1280x1024Data_2;
   SiS_Pr->SiS_LVDS1400x1050Data_1 = (SiS_LVDSDataStruct *)SiS300_LVDS1400x1050Data_1;
   SiS_Pr->SiS_LVDS1400x1050Data_2 = (SiS_LVDSDataStruct *)SiS300_LVDS1400x1050Data_2;
   SiS_Pr->SiS_LVDS1024x600Data_1  = (SiS_LVDSDataStruct *)SiS300_LVDS1024x600Data_1;
   SiS_Pr->SiS_LVDS1024x600Data_2  = (SiS_LVDSDataStruct *)SiS300_LVDS1024x600Data_2;
   SiS_Pr->SiS_LVDS1152x768Data_1  = (SiS_LVDSDataStruct *)SiS300_LVDS1152x768Data_1;
   SiS_Pr->SiS_LVDS1152x768Data_2  = (SiS_LVDSDataStruct *)SiS300_LVDS1152x768Data_2;
   SiS_Pr->SiS_LVDSXXXxXXXData_1   = (SiS_LVDSDataStruct *)SiS300_LVDSXXXxXXXData_1;
   SiS_Pr->SiS_LVDS320x480Data_1   = (SiS_LVDSDataStruct *)SiS300_LVDS320x480Data_1;
   SiS_Pr->SiS_LVDS640x480Data_1   = (SiS_LVDSDataStruct *)SiS300_LVDS640x480Data_1;
   SiS_Pr->SiS_LCDA1400x1050Data_1 = (SiS_LVDSDataStruct *)SiS300_LCDA1400x1050Data_1;
   SiS_Pr->SiS_LCDA1400x1050Data_2 = (SiS_LVDSDataStruct *)SiS300_LCDA1400x1050Data_2;
   SiS_Pr->SiS_LCDA1600x1200Data_1 = (SiS_LVDSDataStruct *)SiS300_LCDA1600x1200Data_1;
   SiS_Pr->SiS_LCDA1600x1200Data_2 = (SiS_LVDSDataStruct *)SiS300_LCDA1600x1200Data_2;
   SiS_Pr->SiS_CHTVUNTSCData = (SiS_LVDSDataStruct *)SiS300_CHTVUNTSCData;
   SiS_Pr->SiS_CHTVONTSCData = (SiS_LVDSDataStruct *)SiS300_CHTVONTSCData;
   SiS_Pr->SiS_CHTVUPALData  = (SiS_LVDSDataStruct *)SiS300_CHTVUPALData;
   SiS_Pr->SiS_CHTVOPALData  = (SiS_LVDSDataStruct *)SiS300_CHTVOPALData;
   SiS_Pr->SiS_PanelType00_1 = (SiS_LVDSDesStruct *)SiS300_PanelType00_1;
   SiS_Pr->SiS_PanelType01_1 = (SiS_LVDSDesStruct *)SiS300_PanelType01_1;
   SiS_Pr->SiS_PanelType02_1 = (SiS_LVDSDesStruct *)SiS300_PanelType02_1;
   SiS_Pr->SiS_PanelType03_1 = (SiS_LVDSDesStruct *)SiS300_PanelType03_1;
   SiS_Pr->SiS_PanelType04_1 = (SiS_LVDSDesStruct *)SiS300_PanelType04_1;
   SiS_Pr->SiS_PanelType05_1 = (SiS_LVDSDesStruct *)SiS300_PanelType05_1;
   SiS_Pr->SiS_PanelType06_1 = (SiS_LVDSDesStruct *)SiS300_PanelType06_1;
   SiS_Pr->SiS_PanelType07_1 = (SiS_LVDSDesStruct *)SiS300_PanelType07_1;
   SiS_Pr->SiS_PanelType08_1 = (SiS_LVDSDesStruct *)SiS300_PanelType08_1;
   SiS_Pr->SiS_PanelType09_1 = (SiS_LVDSDesStruct *)SiS300_PanelType09_1;
   SiS_Pr->SiS_PanelType0a_1 = (SiS_LVDSDesStruct *)SiS300_PanelType0a_1;
   SiS_Pr->SiS_PanelType0b_1 = (SiS_LVDSDesStruct *)SiS300_PanelType0b_1;
   SiS_Pr->SiS_PanelType0c_1 = (SiS_LVDSDesStruct *)SiS300_PanelType0c_1;
   SiS_Pr->SiS_PanelType0d_1 = (SiS_LVDSDesStruct *)SiS300_PanelType0d_1;
   SiS_Pr->SiS_PanelType0e_1 = (SiS_LVDSDesStruct *)SiS300_PanelType0e_1;
   SiS_Pr->SiS_PanelType0f_1 = (SiS_LVDSDesStruct *)SiS300_PanelType0f_1;
   SiS_Pr->SiS_PanelType00_2 = (SiS_LVDSDesStruct *)SiS300_PanelType00_2;
   SiS_Pr->SiS_PanelType01_2 = (SiS_LVDSDesStruct *)SiS300_PanelType01_2;
   SiS_Pr->SiS_PanelType02_2 = (SiS_LVDSDesStruct *)SiS300_PanelType02_2;
   SiS_Pr->SiS_PanelType03_2 = (SiS_LVDSDesStruct *)SiS300_PanelType03_2;
   SiS_Pr->SiS_PanelType04_2 = (SiS_LVDSDesStruct *)SiS300_PanelType04_2;
   SiS_Pr->SiS_PanelType05_2 = (SiS_LVDSDesStruct *)SiS300_PanelType05_2;
   SiS_Pr->SiS_PanelType06_2 = (SiS_LVDSDesStruct *)SiS300_PanelType06_2;
   SiS_Pr->SiS_PanelType07_2 = (SiS_LVDSDesStruct *)SiS300_PanelType07_2;
   SiS_Pr->SiS_PanelType08_2 = (SiS_LVDSDesStruct *)SiS300_PanelType08_2;
   SiS_Pr->SiS_PanelType09_2 = (SiS_LVDSDesStruct *)SiS300_PanelType09_2;
   SiS_Pr->SiS_PanelType0a_2 = (SiS_LVDSDesStruct *)SiS300_PanelType0a_2;
   SiS_Pr->SiS_PanelType0b_2 = (SiS_LVDSDesStruct *)SiS300_PanelType0b_2;
   SiS_Pr->SiS_PanelType0c_2 = (SiS_LVDSDesStruct *)SiS300_PanelType0c_2;
   SiS_Pr->SiS_PanelType0d_2 = (SiS_LVDSDesStruct *)SiS300_PanelType0d_2;
   SiS_Pr->SiS_PanelType0e_2 = (SiS_LVDSDesStruct *)SiS300_PanelType0e_2;
   SiS_Pr->SiS_PanelType0f_2 = (SiS_LVDSDesStruct *)SiS300_PanelType0f_2;
   SiS_Pr->SiS_CHTVUNTSCDesData = (SiS_LVDSDesStruct *)SiS300_CHTVUNTSCDesData;
   SiS_Pr->SiS_CHTVONTSCDesData = (SiS_LVDSDesStruct *)SiS300_CHTVONTSCDesData;
   SiS_Pr->SiS_CHTVUPALDesData  = (SiS_LVDSDesStruct *)SiS300_CHTVUPALDesData;
   SiS_Pr->SiS_CHTVOPALDesData  = (SiS_LVDSDesStruct *)SiS300_CHTVOPALDesData;
   SiS_Pr->SiS_LVDSCRT1800x600_1     = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT1800x600_1;
   SiS_Pr->SiS_LVDSCRT11024x768_1    = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x768_1;
   SiS_Pr->SiS_LVDSCRT11280x1024_1   = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11280x1024_1;
   SiS_Pr->SiS_LVDSCRT11024x600_1    = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x600_1;
   SiS_Pr->SiS_LVDSCRT11152x768_1    = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11152x768_1;
   SiS_Pr->SiS_LVDSCRT1800x600_1_H   = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT1800x600_1_H;
   SiS_Pr->SiS_LVDSCRT11024x768_1_H  = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x768_1_H;
   SiS_Pr->SiS_LVDSCRT11280x1024_1_H = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11280x1024_1_H;
   SiS_Pr->SiS_LVDSCRT11024x600_1_H  = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x600_1_H;
   SiS_Pr->SiS_LVDSCRT11152x768_1_H  = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11152x768_1_H;
   SiS_Pr->SiS_LVDSCRT1800x600_2     = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT1800x600_2;
   SiS_Pr->SiS_LVDSCRT11024x768_2    = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x768_2;
   SiS_Pr->SiS_LVDSCRT11280x1024_2   = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11280x1024_2;
   SiS_Pr->SiS_LVDSCRT11024x600_2    = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x600_2;
   SiS_Pr->SiS_LVDSCRT11152x768_2    = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11152x768_2;
   SiS_Pr->SiS_LVDSCRT1800x600_2_H   = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT1800x600_2_H;
   SiS_Pr->SiS_LVDSCRT11024x768_2_H  = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x768_2_H;
   SiS_Pr->SiS_LVDSCRT11280x1024_2_H = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11280x1024_2_H;
   SiS_Pr->SiS_LVDSCRT11024x600_2_H  = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11024x600_2_H;
   SiS_Pr->SiS_LVDSCRT11152x768_2_H  = (SiS_LVDSCRT1DataStruct *)SiS300_LVDSCRT11152x768_2_H;
   SiS_Pr->SiS_CHTVCRT1UNTSC = (SiS_LVDSCRT1DataStruct *)SiS300_CHTVCRT1UNTSC;
   SiS_Pr->SiS_CHTVCRT1ONTSC = (SiS_LVDSCRT1DataStruct *)SiS300_CHTVCRT1ONTSC;
   SiS_Pr->SiS_CHTVCRT1UPAL  = (SiS_LVDSCRT1DataStruct *)SiS300_CHTVCRT1UPAL;
   SiS_Pr->SiS_CHTVCRT1OPAL  = (SiS_LVDSCRT1DataStruct *)SiS300_CHTVCRT1OPAL;
   SiS_Pr->SiS_CHTVReg_UNTSC = (SiS_CHTVRegDataStruct *)SiS300_CHTVReg_UNTSC;
   SiS_Pr->SiS_CHTVReg_ONTSC = (SiS_CHTVRegDataStruct *)SiS300_CHTVReg_ONTSC;
   SiS_Pr->SiS_CHTVReg_UPAL  = (SiS_CHTVRegDataStruct *)SiS300_CHTVReg_UPAL;
   SiS_Pr->SiS_CHTVReg_OPAL  = (SiS_CHTVRegDataStruct *)SiS300_CHTVReg_OPAL;
   SiS_Pr->SiS_CHTVVCLKUNTSC = SiS300_CHTVVCLKUNTSC;
   SiS_Pr->SiS_CHTVVCLKONTSC = SiS300_CHTVVCLKONTSC;
   SiS_Pr->SiS_CHTVVCLKUPAL  = SiS300_CHTVVCLKUPAL;
   SiS_Pr->SiS_CHTVVCLKOPAL  = SiS300_CHTVVCLKOPAL;

   /* TW: LCDResInfo will on 300 series be translated to 310/325 series definitions */
   SiS_Pr->SiS_Panel320x480   = Panel_320x480;
   SiS_Pr->SiS_Panel640x480   = Panel_640x480;
   SiS_Pr->SiS_Panel800x600   = Panel_800x600;
   SiS_Pr->SiS_Panel1024x768  = Panel_1024x768;
   SiS_Pr->SiS_Panel1280x1024 = Panel_1280x1024;
   SiS_Pr->SiS_Panel1280x960  = Panel_1280x960;
   SiS_Pr->SiS_Panel1024x600  = Panel_1024x600;
   SiS_Pr->SiS_Panel1152x768  = Panel_1152x768;
   SiS_Pr->SiS_Panel1600x1200 = 16;  		/* TW: Something illegal */
   SiS_Pr->SiS_Panel1400x1050 = 16;  		/* TW: Something illegal */
   SiS_Pr->SiS_Panel1152x864  = 16;   		/* TW: Something illegal */
   SiS_Pr->SiS_Panel1280x768  = 16;   		/* TW: Something illegal */
   SiS_Pr->SiS_PanelMax       = Panel_320x480;     /* TW: highest value */
   SiS_Pr->SiS_PanelMinLVDS   = Panel_800x600;     /* TW: Lowest value LVDS */
   SiS_Pr->SiS_PanelMin301    = Panel_1024x768;    /* TW: lowest value 301 */
}
#endif

#ifdef SIS315H
void
InitTo310Pointer(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   SiS_Pr->SiS_SModeIDTable  = (SiS_StStruct *)SiS310_SModeIDTable;
   SiS_Pr->SiS_StandTable    = (SiS_StandTableStruct *)SiS310_StandTable;
   SiS_Pr->SiS_EModeIDTable  = (SiS_ExtStruct *)SiS310_EModeIDTable;
   SiS_Pr->SiS_RefIndex      = (SiS_Ext2Struct *)SiS310_RefIndex;
   SiS_Pr->SiS_CRT1Table     = (SiS_CRT1TableStruct *)SiS310_CRT1Table;
   /* TW: MCLK is different */
   if(HwDeviceExtension->jChipType > SIS_315PRO) {
      SiS_Pr->SiS_MCLKData_0 = (SiS_MCLKDataStruct *)SiS310_MCLKData_0_650;  /* 550, 650 */
   } else {
      SiS_Pr->SiS_MCLKData_0 = (SiS_MCLKDataStruct *)SiS310_MCLKData_0_315;  /* 315 */
   }
   SiS_Pr->SiS_MCLKData_1    = (SiS_MCLKDataStruct *)SiS310_MCLKData_1;
   SiS_Pr->SiS_ECLKData      = (SiS_ECLKDataStruct *)SiS310_ECLKData;
   SiS_Pr->SiS_VCLKData      = (SiS_VCLKDataStruct *)SiS310_VCLKData;
   SiS_Pr->SiS_VBVCLKData    = (SiS_VBVCLKDataStruct *)SiS310_VBVCLKData;
   SiS_Pr->SiS_ScreenOffset  = SiS310_ScreenOffset;
   SiS_Pr->SiS_StResInfo     = (SiS_StResInfoStruct *)SiS310_StResInfo;
   SiS_Pr->SiS_ModeResInfo   = (SiS_ModeResInfoStruct *)SiS310_ModeResInfo;

   SiS_Pr->pSiS_OutputSelect = &SiS310_OutputSelect;
   SiS_Pr->pSiS_SoftSetting  = &SiS310_SoftSetting;

   SiS_Pr->SiS_SR15  = SiS310_SR15;

#ifndef LINUX_XF86
   SiS_Pr->pSiS_SR07 = &SiS310_SR07;
   SiS_Pr->SiS_CR40  = SiS310_CR40;
   SiS_Pr->SiS_CR49  = SiS310_CR49;
   SiS_Pr->pSiS_SR1F = &SiS310_SR1F;
   SiS_Pr->pSiS_SR21 = &SiS310_SR21;
   SiS_Pr->pSiS_SR22 = &SiS310_SR22;
   SiS_Pr->pSiS_SR23 = &SiS310_SR23;
   SiS_Pr->pSiS_SR24 = &SiS310_SR24;
   SiS_Pr->SiS_SR25  = SiS310_SR25;
   SiS_Pr->pSiS_SR31 = &SiS310_SR31;
   SiS_Pr->pSiS_SR32 = &SiS310_SR32;
   SiS_Pr->pSiS_SR33 = &SiS310_SR33;
   SiS_Pr->pSiS_CRT2Data_1_2  = &SiS310_CRT2Data_1_2;
   SiS_Pr->pSiS_CRT2Data_4_D  = &SiS310_CRT2Data_4_D;
   SiS_Pr->pSiS_CRT2Data_4_E  = &SiS310_CRT2Data_4_E;
   SiS_Pr->pSiS_CRT2Data_4_10 = &SiS310_CRT2Data_4_10;
   SiS_Pr->pSiS_RGBSenseData    = &SiS310_RGBSenseData;
   SiS_Pr->pSiS_VideoSenseData  = &SiS310_VideoSenseData;
   SiS_Pr->pSiS_YCSenseData     = &SiS310_YCSenseData;
   SiS_Pr->pSiS_RGBSenseData2   = &SiS310_RGBSenseData2;
   SiS_Pr->pSiS_VideoSenseData2 = &SiS310_VideoSenseData2;
   SiS_Pr->pSiS_YCSenseData2    = &SiS310_YCSenseData2;
#endif

   SiS_Pr->SiS_NTSCPhase    = SiS310_NTSCPhase;
   SiS_Pr->SiS_PALPhase     = SiS310_PALPhase;
   SiS_Pr->SiS_NTSCPhase2   = SiS310_NTSCPhase2;
   SiS_Pr->SiS_PALPhase2    = SiS310_PALPhase2;
   SiS_Pr->SiS_PALMPhase    = SiS310_PALMPhase;
   SiS_Pr->SiS_PALNPhase    = SiS310_PALNPhase;
   SiS_Pr->SiS_PALMPhase2   = SiS310_PALMPhase2;
   SiS_Pr->SiS_PALNPhase2   = SiS310_PALNPhase2;
   SiS_Pr->SiS_SpecialPhase = SiS310_SpecialPhase;

   SiS_Pr->SiS_StLCD1024x768Data    = (SiS_LCDDataStruct *)SiS310_StLCD1024x768Data;
   SiS_Pr->SiS_ExtLCD1024x768Data   = (SiS_LCDDataStruct *)SiS310_ExtLCD1024x768Data;
   SiS_Pr->SiS_St2LCD1024x768Data   = (SiS_LCDDataStruct *)SiS310_St2LCD1024x768Data;
   SiS_Pr->SiS_StLCD1280x1024Data   = (SiS_LCDDataStruct *)SiS310_StLCD1280x1024Data;
   SiS_Pr->SiS_ExtLCD1280x1024Data  = (SiS_LCDDataStruct *)SiS310_ExtLCD1280x1024Data;
   SiS_Pr->SiS_St2LCD1280x1024Data  = (SiS_LCDDataStruct *)SiS310_St2LCD1280x1024Data;
   SiS_Pr->SiS_NoScaleData1024x768  = (SiS_LCDDataStruct *)SiS310_NoScaleData1024x768;
   SiS_Pr->SiS_NoScaleData1280x1024 = (SiS_LCDDataStruct *)SiS310_NoScaleData1280x1024;
   SiS_Pr->SiS_LCD1280x960Data      = (SiS_LCDDataStruct *)SiS310_LCD1280x960Data;
   SiS_Pr->SiS_ExtLCD1400x1050Data  = (SiS_LCDDataStruct *)SiS310_ExtLCD1400x1050Data;
   SiS_Pr->SiS_ExtLCD1600x1200Data  = (SiS_LCDDataStruct *)SiS310_ExtLCD1600x1200Data;
   SiS_Pr->SiS_StLCD1400x1050Data   = (SiS_LCDDataStruct *)SiS310_StLCD1400x1050Data;
   SiS_Pr->SiS_StLCD1600x1200Data   = (SiS_LCDDataStruct *)SiS310_StLCD1600x1200Data;
   SiS_Pr->SiS_NoScaleData1400x1050 = (SiS_LCDDataStruct *)SiS310_NoScaleData1400x1050;
   SiS_Pr->SiS_NoScaleData1600x1200 = (SiS_LCDDataStruct *)SiS310_NoScaleData1600x1200;

   SiS_Pr->SiS_StPALData   = (SiS_TVDataStruct *)SiS310_StPALData;
   SiS_Pr->SiS_ExtPALData  = (SiS_TVDataStruct *)SiS310_ExtPALData;
   SiS_Pr->SiS_StNTSCData  = (SiS_TVDataStruct *)SiS310_StNTSCData;
   SiS_Pr->SiS_ExtNTSCData = (SiS_TVDataStruct *)SiS310_ExtNTSCData;
#ifdef oldHV
   SiS_Pr->SiS_St1HiTVData = (SiS_TVDataStruct *)SiS310_St1HiTVData;
   SiS_Pr->SiS_St2HiTVData = (SiS_TVDataStruct *)SiS310_St2HiTVData;
   SiS_Pr->SiS_ExtHiTVData = (SiS_TVDataStruct *)SiS310_ExtHiTVData;
#endif

   SiS_Pr->SiS_NTSCTiming     = SiS310_NTSCTiming;
   SiS_Pr->SiS_PALTiming      = SiS310_PALTiming;
#ifdef oldHV
   SiS_Pr->SiS_HiTVSt1Timing  = SiS310_HiTVSt1Timing;
   SiS_Pr->SiS_HiTVSt2Timing  = SiS310_HiTVSt2Timing;
   SiS_Pr->SiS_HiTVTextTiming = SiS310_HiTVTextTiming;
   SiS_Pr->SiS_HiTVExtTiming  = SiS310_HiTVExtTiming;
   SiS_Pr->SiS_HiTVGroup3Data = SiS310_HiTVGroup3Data;
   SiS_Pr->SiS_HiTVGroup3Simu = SiS310_HiTVGroup3Simu;
   SiS_Pr->SiS_HiTVGroup3Text = SiS310_HiTVGroup3Text;
#endif

   SiS_Pr->SiS_PanelDelayTbl = (SiS_PanelDelayTblStruct *)SiS310_PanelDelayTbl;
   SiS_Pr->SiS_PanelDelayTblLVDS = (SiS_PanelDelayTblStruct *)SiS310_PanelDelayTblLVDS;

   SiS_Pr->SiS_LVDS800x600Data_1   = (SiS_LVDSDataStruct *)SiS310_LVDS800x600Data_1;
   SiS_Pr->SiS_LVDS800x600Data_2   = (SiS_LVDSDataStruct *)SiS310_LVDS800x600Data_2;
   SiS_Pr->SiS_LVDS1024x768Data_1  = (SiS_LVDSDataStruct *)SiS310_LVDS1024x768Data_1;
   SiS_Pr->SiS_LVDS1024x768Data_2  = (SiS_LVDSDataStruct *)SiS310_LVDS1024x768Data_2;
   SiS_Pr->SiS_LVDS1280x1024Data_1 = (SiS_LVDSDataStruct *)SiS310_LVDS1280x1024Data_1;
   SiS_Pr->SiS_LVDS1280x1024Data_2 = (SiS_LVDSDataStruct *)SiS310_LVDS1280x1024Data_2;
   SiS_Pr->SiS_LVDS1280x960Data_1  = (SiS_LVDSDataStruct *)SiS310_LVDS1280x960Data_1;
   SiS_Pr->SiS_LVDS1280x960Data_2  = (SiS_LVDSDataStruct *)SiS310_LVDS1280x960Data_2;
   SiS_Pr->SiS_LVDS1400x1050Data_1 = (SiS_LVDSDataStruct *)SiS310_LVDS1400x1050Data_1;
   SiS_Pr->SiS_LVDS1400x1050Data_2 = (SiS_LVDSDataStruct *)SiS310_LVDS1400x1050Data_2;
   SiS_Pr->SiS_LVDS1024x600Data_1  = (SiS_LVDSDataStruct *)SiS310_LVDS1024x600Data_1;
   SiS_Pr->SiS_LVDS1024x600Data_2  = (SiS_LVDSDataStruct *)SiS310_LVDS1024x600Data_2;
   SiS_Pr->SiS_LVDS1152x768Data_1  = (SiS_LVDSDataStruct *)SiS310_LVDS1152x768Data_1;
   SiS_Pr->SiS_LVDS1152x768Data_2  = (SiS_LVDSDataStruct *)SiS310_LVDS1152x768Data_2;
   SiS_Pr->SiS_LVDSXXXxXXXData_1   = (SiS_LVDSDataStruct *)SiS310_LVDSXXXxXXXData_1;
   SiS_Pr->SiS_LVDS320x480Data_1   = (SiS_LVDSDataStruct *)SiS310_LVDS320x480Data_1;
   SiS_Pr->SiS_LVDS640x480Data_1   = (SiS_LVDSDataStruct *)SiS310_LVDS640x480Data_1;
   SiS_Pr->SiS_LCDA1400x1050Data_1  = (SiS_LVDSDataStruct *)SiS310_LCDA1400x1050Data_1;
   SiS_Pr->SiS_LCDA1400x1050Data_2  = (SiS_LVDSDataStruct *)SiS310_LCDA1400x1050Data_2;
   SiS_Pr->SiS_LCDA1600x1200Data_1  = (SiS_LVDSDataStruct *)SiS310_LCDA1600x1200Data_1;
   SiS_Pr->SiS_LCDA1600x1200Data_2  = (SiS_LVDSDataStruct *)SiS310_LCDA1600x1200Data_2;
   SiS_Pr->SiS_CHTVUNTSCData = (SiS_LVDSDataStruct *)SiS310_CHTVUNTSCData;
   SiS_Pr->SiS_CHTVONTSCData = (SiS_LVDSDataStruct *)SiS310_CHTVONTSCData;
   SiS_Pr->SiS_CHTVUPALData  = (SiS_LVDSDataStruct *)SiS310_CHTVUPALData;
   SiS_Pr->SiS_CHTVOPALData  = (SiS_LVDSDataStruct *)SiS310_CHTVOPALData;
   SiS_Pr->SiS_PanelType00_1 = (SiS_LVDSDesStruct *)SiS310_PanelType00_1;
   SiS_Pr->SiS_PanelType01_1 = (SiS_LVDSDesStruct *)SiS310_PanelType01_1;
   SiS_Pr->SiS_PanelType02_1 = (SiS_LVDSDesStruct *)SiS310_PanelType02_1;
   SiS_Pr->SiS_PanelType03_1 = (SiS_LVDSDesStruct *)SiS310_PanelType03_1;
   SiS_Pr->SiS_PanelType04_1 = (SiS_LVDSDesStruct *)SiS310_PanelType04_1;
   SiS_Pr->SiS_PanelType05_1 = (SiS_LVDSDesStruct *)SiS310_PanelType05_1;
   SiS_Pr->SiS_PanelType06_1 = (SiS_LVDSDesStruct *)SiS310_PanelType06_1;
   SiS_Pr->SiS_PanelType07_1 = (SiS_LVDSDesStruct *)SiS310_PanelType07_1;
   SiS_Pr->SiS_PanelType08_1 = (SiS_LVDSDesStruct *)SiS310_PanelType08_1;
   SiS_Pr->SiS_PanelType09_1 = (SiS_LVDSDesStruct *)SiS310_PanelType09_1;
   SiS_Pr->SiS_PanelType0a_1 = (SiS_LVDSDesStruct *)SiS310_PanelType0a_1;
   SiS_Pr->SiS_PanelType0b_1 = (SiS_LVDSDesStruct *)SiS310_PanelType0b_1;
   SiS_Pr->SiS_PanelType0c_1 = (SiS_LVDSDesStruct *)SiS310_PanelType0c_1;
   SiS_Pr->SiS_PanelType0d_1 = (SiS_LVDSDesStruct *)SiS310_PanelType0d_1;
   SiS_Pr->SiS_PanelType0e_1 = (SiS_LVDSDesStruct *)SiS310_PanelType0e_1;
   SiS_Pr->SiS_PanelType0f_1 = (SiS_LVDSDesStruct *)SiS310_PanelType0f_1;
   SiS_Pr->SiS_PanelType00_2 = (SiS_LVDSDesStruct *)SiS310_PanelType00_2;
   SiS_Pr->SiS_PanelType01_2 = (SiS_LVDSDesStruct *)SiS310_PanelType01_2;
   SiS_Pr->SiS_PanelType02_2 = (SiS_LVDSDesStruct *)SiS310_PanelType02_2;
   SiS_Pr->SiS_PanelType03_2 = (SiS_LVDSDesStruct *)SiS310_PanelType03_2;
   SiS_Pr->SiS_PanelType04_2 = (SiS_LVDSDesStruct *)SiS310_PanelType04_2;
   SiS_Pr->SiS_PanelType05_2 = (SiS_LVDSDesStruct *)SiS310_PanelType05_2;
   SiS_Pr->SiS_PanelType06_2 = (SiS_LVDSDesStruct *)SiS310_PanelType06_2;
   SiS_Pr->SiS_PanelType07_2 = (SiS_LVDSDesStruct *)SiS310_PanelType07_2;
   SiS_Pr->SiS_PanelType08_2 = (SiS_LVDSDesStruct *)SiS310_PanelType08_2;
   SiS_Pr->SiS_PanelType09_2 = (SiS_LVDSDesStruct *)SiS310_PanelType09_2;
   SiS_Pr->SiS_PanelType0a_2 = (SiS_LVDSDesStruct *)SiS310_PanelType0a_2;
   SiS_Pr->SiS_PanelType0b_2 = (SiS_LVDSDesStruct *)SiS310_PanelType0b_2;
   SiS_Pr->SiS_PanelType0c_2 = (SiS_LVDSDesStruct *)SiS310_PanelType0c_2;
   SiS_Pr->SiS_PanelType0d_2 = (SiS_LVDSDesStruct *)SiS310_PanelType0d_2;
   SiS_Pr->SiS_PanelType0e_2 = (SiS_LVDSDesStruct *)SiS310_PanelType0e_2;
   SiS_Pr->SiS_PanelType0f_2 = (SiS_LVDSDesStruct *)SiS310_PanelType0f_2;

   SiS_Pr->LVDS1024x768Des_1  = (SiS_LVDSDesStruct *)SiS310_PanelType1076_1;
   SiS_Pr->LVDS1280x1024Des_1 = (SiS_LVDSDesStruct *)SiS310_PanelType1210_1;
   SiS_Pr->LVDS1400x1050Des_1 = (SiS_LVDSDesStruct *)SiS310_PanelType1296_1 ;
   SiS_Pr->LVDS1600x1200Des_1 = (SiS_LVDSDesStruct *)SiS310_PanelType1600_1 ;
   SiS_Pr->LVDS1024x768Des_2  = (SiS_LVDSDesStruct *)SiS310_PanelType1076_2;
   SiS_Pr->LVDS1280x1024Des_2 = (SiS_LVDSDesStruct *)SiS310_PanelType1210_2;
   SiS_Pr->LVDS1400x1050Des_2 = (SiS_LVDSDesStruct *)SiS310_PanelType1296_2;
   SiS_Pr->LVDS1600x1200Des_2 = (SiS_LVDSDesStruct *)SiS310_PanelType1600_2 ;

   /* TW: New from 650/301LV BIOS */
   SiS_Pr->SiS_CRT2Part2_1024x768_1  = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1024x768_1;
   SiS_Pr->SiS_CRT2Part2_1280x1024_1 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1280x1024_1;
   SiS_Pr->SiS_CRT2Part2_1400x1050_1 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1400x1050_1;
   SiS_Pr->SiS_CRT2Part2_1600x1200_1 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1600x1200_1;
   SiS_Pr->SiS_CRT2Part2_1024x768_2  = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1024x768_2;
   SiS_Pr->SiS_CRT2Part2_1280x1024_2 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1280x1024_2;
   SiS_Pr->SiS_CRT2Part2_1400x1050_2 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1400x1050_2;
   SiS_Pr->SiS_CRT2Part2_1600x1200_2 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1600x1200_2;
   SiS_Pr->SiS_CRT2Part2_1024x768_3  = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1024x768_3;
   SiS_Pr->SiS_CRT2Part2_1280x1024_3 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1280x1024_3;
   SiS_Pr->SiS_CRT2Part2_1400x1050_3 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1400x1050_3;
   SiS_Pr->SiS_CRT2Part2_1600x1200_3 = (SiS_Part2PortTblStruct *)SiS310_CRT2Part2_1600x1200_3;

   SiS_Pr->SiS_CHTVUNTSCDesData = (SiS_LVDSDesStruct *)SiS310_CHTVUNTSCDesData;
   SiS_Pr->SiS_CHTVONTSCDesData = (SiS_LVDSDesStruct *)SiS310_CHTVONTSCDesData;
   SiS_Pr->SiS_CHTVUPALDesData  = (SiS_LVDSDesStruct *)SiS310_CHTVUPALDesData;
   SiS_Pr->SiS_CHTVOPALDesData  = (SiS_LVDSDesStruct *)SiS310_CHTVOPALDesData;

   SiS_Pr->SiS_LVDSCRT1800x600_1     = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT1800x600_1;
   SiS_Pr->SiS_LVDSCRT11024x768_1    = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x768_1;
   SiS_Pr->SiS_LVDSCRT11280x1024_1   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11280x1024_1;
   SiS_Pr->SiS_LVDSCRT11400x1050_1   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11400x1050_1;
   SiS_Pr->SiS_LVDSCRT11024x600_1    = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x600_1;
   SiS_Pr->SiS_LVDSCRT11152x768_1    = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11152x768_1;
   SiS_Pr->SiS_LVDSCRT11600x1200_1   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11600x1200_1;
   SiS_Pr->SiS_LVDSCRT1800x600_1_H   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT1800x600_1_H;
   SiS_Pr->SiS_LVDSCRT11024x768_1_H  = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x768_1_H;
   SiS_Pr->SiS_LVDSCRT11280x1024_1_H = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11280x1024_1_H;
   SiS_Pr->SiS_LVDSCRT11400x1050_1_H = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11400x1050_1_H;
   SiS_Pr->SiS_LVDSCRT11024x600_1_H  = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x600_1_H;
   SiS_Pr->SiS_LVDSCRT11152x768_1_H  = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11152x768_1_H;
   SiS_Pr->SiS_LVDSCRT11600x1200_1_H = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11600x1200_1_H;
   SiS_Pr->SiS_LVDSCRT1800x600_2     = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT1800x600_2;
   SiS_Pr->SiS_LVDSCRT11024x768_2    = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x768_2;
   SiS_Pr->SiS_LVDSCRT11280x1024_2   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11280x1024_2;
   SiS_Pr->SiS_LVDSCRT11400x1050_2   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11400x1050_2;
   SiS_Pr->SiS_LVDSCRT11024x600_2    = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x600_2;
   SiS_Pr->SiS_LVDSCRT11152x768_2    = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11152x768_2;
   SiS_Pr->SiS_LVDSCRT11600x1200_2   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11600x1200_2;
   SiS_Pr->SiS_LVDSCRT1800x600_2_H   = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT1800x600_2_H;
   SiS_Pr->SiS_LVDSCRT11024x768_2_H  = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x768_2_H;
   SiS_Pr->SiS_LVDSCRT11280x1024_2_H = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11280x1024_2_H;
   SiS_Pr->SiS_LVDSCRT11400x1050_2_H = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11400x1050_2_H;
   SiS_Pr->SiS_LVDSCRT11024x600_2_H  = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11024x600_2_H;
   SiS_Pr->SiS_LVDSCRT11152x768_2_H  = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11152x768_2_H;
   SiS_Pr->SiS_LVDSCRT11600x1200_2_H = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT11600x1200_2_H;
   SiS_Pr->SiS_LVDSCRT1XXXxXXX_1     = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT1XXXxXXX_1;
   SiS_Pr->SiS_LVDSCRT1320x480_1     = (SiS_LVDSCRT1DataStruct *)SiS310_LVDSCRT1320x480_1;
   SiS_Pr->SiS_CHTVCRT1UNTSC = (SiS_LVDSCRT1DataStruct *)SiS310_CHTVCRT1UNTSC;
   SiS_Pr->SiS_CHTVCRT1ONTSC = (SiS_LVDSCRT1DataStruct *)SiS310_CHTVCRT1ONTSC;
   SiS_Pr->SiS_CHTVCRT1UPAL  = (SiS_LVDSCRT1DataStruct *)SiS310_CHTVCRT1UPAL;
   SiS_Pr->SiS_CHTVCRT1OPAL  = (SiS_LVDSCRT1DataStruct *)SiS310_CHTVCRT1OPAL;
   SiS_Pr->SiS_CHTVReg_UNTSC = (SiS_CHTVRegDataStruct *)SiS310_CHTVReg_UNTSC;
   SiS_Pr->SiS_CHTVReg_ONTSC = (SiS_CHTVRegDataStruct *)SiS310_CHTVReg_ONTSC;
   SiS_Pr->SiS_CHTVReg_UPAL  = (SiS_CHTVRegDataStruct *)SiS310_CHTVReg_UPAL;
   SiS_Pr->SiS_CHTVReg_OPAL  = (SiS_CHTVRegDataStruct *)SiS310_CHTVReg_OPAL;
   SiS_Pr->SiS_LCDACRT1800x600_1     = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT1800x600_1;
   SiS_Pr->SiS_LCDACRT11024x768_1    = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11024x768_1;
   SiS_Pr->SiS_LCDACRT11280x1024_1   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11280x1024_1;
   SiS_Pr->SiS_LCDACRT11400x1050_1   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11400x1050_1;
   SiS_Pr->SiS_LCDACRT11600x1200_1   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11600x1200_1;
   SiS_Pr->SiS_LCDACRT1800x600_1_H   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT1800x600_1_H;
   SiS_Pr->SiS_LCDACRT11024x768_1_H  = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11024x768_1_H;
   SiS_Pr->SiS_LCDACRT11280x1024_1_H = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11280x1024_1_H;
   SiS_Pr->SiS_LCDACRT11400x1050_1_H = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11400x1050_1_H;
   SiS_Pr->SiS_LCDACRT11600x1200_1_H = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11600x1200_1_H;
   SiS_Pr->SiS_LCDACRT1800x600_2     = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT1800x600_2;
   SiS_Pr->SiS_LCDACRT11024x768_2    = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11024x768_2;
   SiS_Pr->SiS_LCDACRT11280x1024_2   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11280x1024_2;
   SiS_Pr->SiS_LCDACRT11400x1050_2   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11400x1050_2;
   SiS_Pr->SiS_LCDACRT11600x1200_2   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11600x1200_2;
   SiS_Pr->SiS_LCDACRT1800x600_2_H   = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT1800x600_2_H;
   SiS_Pr->SiS_LCDACRT11024x768_2_H  = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11024x768_2_H;
   SiS_Pr->SiS_LCDACRT11280x1024_2_H = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11280x1024_2_H;
   SiS_Pr->SiS_LCDACRT11400x1050_2_H = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11400x1050_2_H;
   SiS_Pr->SiS_LCDACRT11600x1200_2_H = (SiS_LCDACRT1DataStruct *)SiS310_LCDACRT11600x1200_2_H;
   SiS_Pr->SiS_CHTVVCLKUNTSC = SiS310_CHTVVCLKUNTSC;
   SiS_Pr->SiS_CHTVVCLKONTSC = SiS310_CHTVVCLKONTSC;
   SiS_Pr->SiS_CHTVVCLKUPAL  = SiS310_CHTVVCLKUPAL;
   SiS_Pr->SiS_CHTVVCLKOPAL  = SiS310_CHTVVCLKOPAL;

   SiS_Pr->SiS_Panel320x480   = Panel_320x480;
   SiS_Pr->SiS_Panel640x480   = Panel_640x480;
   SiS_Pr->SiS_Panel800x600   = Panel_800x600;
   SiS_Pr->SiS_Panel1024x768  = Panel_1024x768;
   SiS_Pr->SiS_Panel1280x1024 = Panel_1280x1024;
   SiS_Pr->SiS_Panel1280x960  = Panel_1280x960;
   SiS_Pr->SiS_Panel1600x1200 = Panel_1600x1200;
   SiS_Pr->SiS_Panel1400x1050 = Panel_1400x1050;
   SiS_Pr->SiS_Panel1152x768  = Panel_1152x768;
   SiS_Pr->SiS_Panel1152x864  = Panel_1152x864;
   SiS_Pr->SiS_Panel1280x768  = Panel_1280x768;
   SiS_Pr->SiS_Panel1024x600  = Panel_1024x600;
   SiS_Pr->SiS_PanelMax       = Panel_320x480;    /* TW: highest value */
   SiS_Pr->SiS_PanelMinLVDS   = Panel_800x600;    /* TW: lowest value LVDS/LCDA */
   SiS_Pr->SiS_PanelMin301    = Panel_1024x768;   /* TW: lowest value 301 */
}
#endif

#ifdef LINUXBIOS
/* -------------- SiSInit -----------------*/
/* TW: I degraded this for LINUXBIOS only, because we
 *     don't need this otherwise
 */
BOOLEAN
SiSInit(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   UCHAR  *ROMAddr  = HwDeviceExtension->pjVirtualRomBase;
   ULONG   FBAddr   = (ULONG)HwDeviceExtension->pjVideoMemoryAddress;
   USHORT  BaseAddr = (USHORT)HwDeviceExtension->ulIOAddress;
   UCHAR   i, temp=0;
   UCHAR   SR11;
#ifdef LINUX_KERNEL
   UCHAR   temp1;
   ULONG   base;
#endif
   UCHAR   SR13=0, SR14=0, SR16=0
   UCHAR   SR17=0, SR19=0, SR1A=0;
#ifdef SIS300
   UCHAR   SR18=0, SR12=0;
#endif
#ifdef SIS315H
   UCHAR   CR37=0, CR38=0, CR79=0,
   UCHAR   CR7A=0, CR7B=0, CR7C=0;
   UCHAR   SR1B=0, SR15=0;
   PSIS_DSReg pSR;
   ULONG   Temp;
#endif
   UCHAR   VBIOSVersion[5];

   if(FBAddr==0)    return (FALSE);
   if(BaseAddr==0)  return (FALSE);

   SiS_SetReg3((USHORT)(BaseAddr+0x12),  0x67);  /* Misc */

#ifdef SIS315H
   if(HwDeviceExtension->jChipType > SIS_315PRO) {
     if(!HwDeviceExtension->bIntegratedMMEnabled)
     	return (FALSE);
   }
#endif

   SiS_MemoryCopy(VBIOSVersion,HwDeviceExtension->szVBIOSVer,4);
   VBIOSVersion[4]= 0x00;

   SiSDetermineROMUsage(SiS_Pr, HwDeviceExtension, ROMAddr);

   /* TW: Init pointers */
#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_315H) ||
      (HwDeviceExtension->jChipType == SIS_315PRO) ||
      (HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650))
     InitTo310Pointer(SiS_Pr, HwDeviceExtension);
#endif

#ifdef SIS300
   if((HwDeviceExtension->jChipType == SIS_540) ||
      (HwDeviceExtension->jChipType == SIS_630) ||
      (HwDeviceExtension->jChipType == SIS_730) ||
      (HwDeviceExtension->jChipType == SIS_300))
     InitTo300Pointer(SiS_Pr, HwDeviceExtension);
#endif

   /* TW: Set SiS Register definitions */
   SiSRegInit(SiS_Pr, BaseAddr);

   /* TW: Determine LVDS/CH70xx/TRUMPION */
   SiS_Set_LVDS_TRUMPION(SiS_Pr, HwDeviceExtension);

   /* TW: Unlock registers */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x05,0x86);

#ifdef LINUX_KERNEL

#ifdef SIS300                                         	    /* Set SR14 */
   if((HwDeviceExtension->jChipType==SIS_540) ||
      (HwDeviceExtension->jChipType==SIS_630) ||
      (HwDeviceExtension->jChipType==SIS_730)) {
     base=0x80000060;
     OutPortLong(base,0xcf8);
     temp1 = InPortLong(0xcfc);
     temp1 >>= (16+8+4);
     temp1 &= 0x07;
     temp1++;
     temp1 = 1 << temp1;
     SR14 = temp1 - 1;
     base = 0x80000064;
     OutPortLong(base,0xcf8);
     temp1 = InPortLong(0xcfc);
     temp1 &= 0x00000020;
     if(temp1) 	SR14 |= 0x80;
     else      	SR14 |= 0x40;
   }
#endif

#ifdef SIS315H                                              /* Set SR14 */
   if(HwDeviceExtension->jChipType==SIS_550) {
     base = 0x80000060;
     OutPortLong(base,0xcf8);
     temp1 = InPortLong(0xcfc);
     temp1 >>= (16+8+4);
     temp1 &= 0x07;
     temp1++;
     temp1 = 1 << temp1;
     SR14 = temp1 - 1;
     base = 0x80000064;
     OutPortLong(base,0xcf8);
     temp1 = InPortLong(0xcfc);
     temp1 &= 0x00000020;
     if(temp1)  SR14 |= 0x80;
     else       SR14 |= 0x40;
   }

   if((HwDeviceExtension->jChipType == SIS_640) ||      /* Set SR14 */
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650)) {
     base = 0x80000064;
     OutPortLong(base,0xcf8);
     temp1=InPortLong(0xcfc);
     temp1 >>= 4;
     temp1 &= 0x07;
     if(temp1 > 2) {
       temp = temp1;
       switch(temp) {
        case 3: temp1 = 0x07;  break;
        case 4: temp1 = 0x0F;  break;
        case 5: temp1 = 0x1F;  break;
        case 6: temp1 = 0x05;  break;
        case 7: temp1 = 0x17;  break;
        case 8: break;
        case 9: break;
       }
     }
     SR14 = temp1;
     base = 0x8000007C;
     OutPortLong(base,0xcf8);
     temp1 = InPortLong(0xcfc);
     temp1 &= 0x00000020;
     if(temp1)  SR14 |= 0x80;
   }
#endif

#endif  /* Linux kernel */

#ifdef SIS300
   if((HwDeviceExtension->jChipType == SIS_540)||
      (HwDeviceExtension->jChipType == SIS_630)||
      (HwDeviceExtension->jChipType == SIS_730)) {
     SR12 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x12);
     SR13 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x13);
     SR14 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
     SR16 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x16);
     SR17 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x17);
     SR18 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x18);
     SR19 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x19);
     SR1A = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1A);
   } else if(HwDeviceExtension->jChipType == SIS_300){
     SR13 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x13);
     SR14 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
   }
#endif
#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650)) {
     SR19 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x19);
     SR19 = (SR19)||0x01;  /* TW: ??? || ??? */
     if(SR19==0x00) {
     	SR13 = 0x22;
     	SR14 = 0x00;
    	SR15 = 0x01;
     	SR16 = 0x00;
     	SR17 = 0x00;
     	SR1A = 0x00;
     	SR1B = 0x00;
     	CR37 = 0x00;
     	CR38 = 0x00;
     	CR79 = 0x00;
     	CR7A = 0x00;
     	CR7B = 0x00;
     	CR7C = 0x00;
     } else {
     	SR13 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x13);
     	SR14 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
     	SR15 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x15);
     	SR16 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x16);
     	SR17 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x17);
     	SR1A = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1A);
     	SR1B = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1B);
     	CR37 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3d4,0x37);  /* TW: Was 0x02 - why? */
     	CR38 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3d4,0x38);
     	CR79 = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3d4,0x79);
     	CR7A = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3d4,0x7A);
     	CR7B = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3d4,0x7B);
     	CR7C = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3d4,0x7C);
     }
   }
#endif

   /* Reset extended registers */

   for(i=0x06; i< 0x20; i++) SiS_SetReg1(SiS_Pr->SiS_P3c4,i,0);
   for(i=0x21; i<=0x27; i++) SiS_SetReg1(SiS_Pr->SiS_P3c4,i,0);
   for(i=0x31; i<=0x3D; i++) SiS_SetReg1(SiS_Pr->SiS_P3c4,i,0);

#ifdef SIS300
   if((HwDeviceExtension->jChipType == SIS_540) ||
      (HwDeviceExtension->jChipType == SIS_630) ||
      (HwDeviceExtension->jChipType == SIS_730) ||
      (HwDeviceExtension->jChipType == SIS_300)) {
     	for(i=0x38; i<=0x3F; i++) SiS_SetReg1(SiS_Pr->SiS_P3d4,i,0);
   }
#endif

#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_315H) ||
      (HwDeviceExtension->jChipType == SIS_315PRO) ||
      (HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650)) {
   	for(i=0x12; i<=0x1B; i++) SiS_SetReg1(SiS_Pr->SiS_P3c4,i,0);
   	for(i=0x79; i<=0x7C; i++) SiS_SetReg1(SiS_Pr->SiS_P3d4,i,0);
   }
#endif

   /* Restore Extended Registers */

#ifdef SIS300
   if((HwDeviceExtension->jChipType == SIS_540) ||
      (HwDeviceExtension->jChipType == SIS_630) ||
      (HwDeviceExtension->jChipType == SIS_730)) {
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x12,SR12);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,SR13);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,SR14);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,SR16);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x17,SR17);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x18,SR18);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x19,SR19);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x1A,SR1A);
   }
#endif

#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650)) {
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,SR13);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,SR14);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x15,SR15);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,SR16);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x17,SR17);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x19,SR19);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x1A,SR1A);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x1B,SR1B);
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x37,CR37);
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x38,CR38);
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x79,CR79);
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x7A,CR7A);
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x7B,CR7B);
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x7C,CR7C);
   }
#endif

#ifdef SIS300
   if((HwDeviceExtension->jChipType==SIS_540) ||
      (HwDeviceExtension->jChipType==SIS_630) ||
      (HwDeviceExtension->jChipType==SIS_730)) {
     	temp = (UCHAR)SR1A & 0x03;
   } else if(HwDeviceExtension->jChipType==SIS_300) {
        /* TW: Nothing */
   }
#endif
#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_315H )||
      (HwDeviceExtension->jChipType == SIS_315PRO)) {
      	if((*SiS_Pr->pSiS_SoftSetting & SoftDRAMType) == 0){
          	temp = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x3A) & 0x03;
        }
   }
   if((HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650)) {
        if((*SiS_Pr->pSiS_SoftSetting & SoftDRAMType) == 0){
          	temp = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x13) & 0x07;
        }
   }
#endif

   SiS_Pr->SiS_RAMType = temp;
   SiS_SetMemoryClock(SiS_Pr, ROMAddr, HwDeviceExtension);

   /* Set default register contents */

   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x07,*SiS_Pr->pSiS_SR07); 		/* DAC speed */

   if((HwDeviceExtension->jChipType != SIS_540) &&
      (HwDeviceExtension->jChipType != SIS_630) &&
      (HwDeviceExtension->jChipType != SIS_730)){
     	for(i=0x15;i<0x1C;i++) {
       	    SiS_SetReg1(SiS_Pr->SiS_P3c4,i,SiS_Pr->SiS_SR15[i-0x15][SiS_Pr->SiS_RAMType]);
     	}
   }

#ifdef SIS315H
   if ((HwDeviceExtension->jChipType == SIS_315H ) ||
       (HwDeviceExtension->jChipType == SIS_315PRO)) {
     	for(i=0x40;i<=0x44;i++) {
       	    SiS_SetReg1(SiS_Pr->SiS_P3d4,i,SiS_Pr->SiS_CR40[i-0x40][SiS_Pr->SiS_RAMType]);
     	}
     	SiS_SetReg1(SiS_Pr->SiS_P3d4,0x48,0x23);
     	SiS_SetReg1(SiS_Pr->SiS_P3d4,0x49,SiS_Pr->SiS_CR49[0]);
    /*  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x25,SiS_Pr->SiS_SR25[0]);  */
   }
#endif

   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x1F,*SiS_Pr->pSiS_SR1F); 	/* DAC pedestal */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x20,0xA0);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x23,*SiS_Pr->pSiS_SR23);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x24,*SiS_Pr->pSiS_SR24);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x25,SiS_Pr->SiS_SR25[0]);

#ifdef SIS300
   if(HwDeviceExtension->jChipType == SIS_300) {
     	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x21,0x84);
     	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x22,0x00);
   }
#endif

   SR11 = 0x0F;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x11,SR11);		/* Power Management & DDC port */

   SiS_UnLockCRT2(SiS_Pr, HwDeviceExtension, BaseAddr);
   SiS_SetReg1(SiS_Pr->SiS_Part1Port,0x00,0x00);
   SiS_SetReg1(SiS_Pr->SiS_Part1Port,0x02,*SiS_Pr->pSiS_CRT2Data_1_2);

#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_315H) ||
      (HwDeviceExtension->jChipType == SIS_315PRO) ||
      (HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650))
     	SiS_SetReg1(SiS_Pr->SiS_Part1Port,0x2E,0x08);    /* use VB */
#endif

   temp = *SiS_Pr->pSiS_SR32;
   if(SiS_BridgeIsOn(SiS_Pr, BaseAddr)) {
     	temp &= 0xEF;
   }
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x32,temp);

#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_315H) ||
      (HwDeviceExtension->jChipType == SIS_315PRO)) {
     HwDeviceExtension->pQueryVGAConfigSpace(HwDeviceExtension,0x50,0,&Temp);
     Temp >>= 20;
     Temp &= 0xF;
     if (Temp != 1) {
     	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x25,SiS_Pr->SiS_SR25[1]);
     	SiS_SetReg1(SiS_Pr->SiS_P3d4,0x49,SiS_Pr->SiS_CR49[1]);
     }

     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x27,0x1F);

     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x31,*SiS_Pr->pSiS_SR31);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x32,*SiS_Pr->pSiS_SR32);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x33,*SiS_Pr->pSiS_SR33);
   }
#endif

   if (SiS_BridgeIsOn(SiS_Pr, BaseAddr) == 0) {
     	if(SiS_Pr->SiS_IF_DEF_LVDS == 0) {
       		SiS_SetReg1(SiS_Pr->SiS_Part2Port,0x00,0x1C);
       		SiS_SetReg1(SiS_Pr->SiS_Part4Port,0x0D,*SiS_Pr->pSiS_CRT2Data_4_D);
       		SiS_SetReg1(SiS_Pr->SiS_Part4Port,0x0E,*SiS_Pr->pSiS_CRT2Data_4_E);
       		SiS_SetReg1(SiS_Pr->SiS_Part4Port,0x10,*SiS_Pr->pSiS_CRT2Data_4_10);
       		SiS_SetReg1(SiS_Pr->SiS_Part4Port,0x0F,0x3F);
     	}
     	SiS_LockCRT2(SiS_Pr, HwDeviceExtension, BaseAddr);
   }
   SiS_SetReg1(SiS_Pr->SiS_P3d4,0x83,0x00);

#ifdef SIS315H
   if ((HwDeviceExtension->jChipType==SIS_315H) ||
       (HwDeviceExtension->jChipType==SIS_315PRO)) {
       	if (HwDeviceExtension->bSkipDramSizing==TRUE) {
         	SiS_SetDRAMModeRegister(SiS_Pr, ROMAddr,HwDeviceExtension);
         	pSR = HwDeviceExtension->pSR;
         	if (pSR!=NULL) {
           		while (pSR->jIdx!=0xFF) {
             			SiS_SetReg1(SiS_Pr->SiS_P3c4,pSR->jIdx,pSR->jVal);
             			pSR++;
           		}
         	}
       } else SiS_SetDRAMSize_310(SiS_Pr, HwDeviceExtension);
   }
#endif

#ifdef SIS315H
   if((HwDeviceExtension->jChipType==SIS_550)){
       /* SetDRAMConfig begin */
/*     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x12,SR12);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,SR13);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,SR14);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,SR16);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x17,SR17);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x18,SR18);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x19,SR19);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x1A,SR1A);   */
       /* SetDRAMConfig end */
   }
#endif

#ifdef SIS300
   if(HwDeviceExtension->jChipType == SIS_300) {
       	if (HwDeviceExtension->bSkipDramSizing == TRUE) {
/*       	SiS_SetDRAMModeRegister(ROMAddr,HwDeviceExtension);
         	temp = (HwDeviceExtension->pSR)->jVal;
         	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,temp);
         	temp = (HwDeviceExtension->pSR)->jVal;
         	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,temp);   */
       } else {
#ifdef TC
         	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,SR13);
         	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,SR14);
         	SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x15,0xFF,0x04);
#else
         	SiS_SetDRAMSize_300(SiS_Pr, HwDeviceExtension);
         	SiS_SetDRAMSize_300(SiS_Pr, HwDeviceExtension);
#endif
       }
   }
   if((HwDeviceExtension->jChipType==SIS_540)||
      (HwDeviceExtension->jChipType==SIS_630)||
      (HwDeviceExtension->jChipType==SIS_730)) {
   }
/* SetDRAMSize end */
#endif /* SIS300 */

   /* Set default Ext2Regs */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x21,*SiS_Pr->pSiS_SR21);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x22,*SiS_Pr->pSiS_SR22);


#ifdef LINUXBIOS       /* TW: This is not needed for our purposes */
   SiS_DetectMonitor(SiS_Pr, HwDeviceExtension,BaseAddr);    /* Sense CRT1 */
   SiS_GetSenseStatus(SiS_Pr, HwDeviceExtension,ROMAddr);    /* Sense CRT2 */
#endif

   return(TRUE);
}

void
SiS_Set_LVDS_TRUMPION(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  USHORT temp = 0;

#ifdef SiS300
  if((HwDeviceExtension->jChipType == SIS_540) ||
     (HwDeviceExtension->jChipType == SIS_630) ||
     (HwDeviceExtension->jChipType == SIS_730)) {
        /* TW: Read POWER_ON_TRAP and copy to CR37 */
    	temp = (UCHAR)SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1A);
    	temp = (temp & 0xE0) >> 4;
   	SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x37,0xF1,temp);
  }
#endif
#ifdef SIS315H
  if((HwDeviceExtension->jChipType == SIS_640) ||
     (HwDeviceExtension->jChipType == SIS_740) ||
     (HwDeviceExtension->jChipType == SIS_650)) {
  }
#endif

   SiSSetLVDSetc(SiS_Pr, HwDeviceExtension, 0);
}

/* ===============  SiS 300 dram sizing begin  =============== */
#ifdef SIS300
void
SiS_SetDRAMSize_300(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   ULONG   FBAddr = (ULONG)HwDeviceExtension->pjVideoMemoryAddress;
   USHORT  SR13, SR14=0, buswidth, Done;
   SHORT   i, j, k;
   USHORT  data, TotalCapacity, PhysicalAdrOtherPage=0;
   ULONG   Addr;
   UCHAR   temp;
   int     PseudoRankCapacity, PseudoTotalCapacity, PseudoAdrPinCount;
   int     RankCapacity, AdrPinCount, BankNumHigh, BankNumMid, MB2Bank;
   int     PageCapacity, PhysicalAdrHigh, PhysicalAdrHalfPage;

   SiSSetMode(SiS_Pr, HwDeviceExtension, 0x2e);

   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x01,0x20);        /* Turn OFF Display  */

   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,0x00);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,0xBF);

   buswidth = SiS_ChkBUSWidth_300(SiS_Pr, FBAddr);

   MB2Bank = 16;
   Done = 0;
   for(i=6; i>=0; i--) {
      if(Done == 1) break;
      PseudoRankCapacity = 1 << i;
      for(j=4; j>=1; j--) {
         if(Done == 1) break;
         PseudoTotalCapacity = PseudoRankCapacity * j;
         PseudoAdrPinCount = 15 - j;
         if(PseudoTotalCapacity <= 64) {
            for(k=0; k<=16; k++) {
               if(Done == 1) break;
               RankCapacity = buswidth * SiS_DRAMType[k][3];
               AdrPinCount = SiS_DRAMType[k][2] + SiS_DRAMType[k][0];
               if(RankCapacity == PseudoRankCapacity)
                 if(AdrPinCount <= PseudoAdrPinCount) {
                    if(j == 3) {             /* Rank No */
                       BankNumHigh = RankCapacity * MB2Bank * 3 - 1;
                       BankNumMid = RankCapacity * MB2Bank * 1 - 1;
                    } else {
                       BankNumHigh = RankCapacity * MB2Bank * j - 1;
                       BankNumMid = RankCapacity * MB2Bank * j / 2 - 1;
                    }
                    PageCapacity = (1 << SiS_DRAMType[k][1]) * buswidth * 4;
                    PhysicalAdrHigh = BankNumHigh;
                    PhysicalAdrHalfPage = (PageCapacity / 2 + PhysicalAdrHigh) % PageCapacity;
                    PhysicalAdrOtherPage = PageCapacity * SiS_DRAMType[k][2] + PhysicalAdrHigh;
                    /* Write data */
                    /*Test*/
                    SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x15,0xFB);
                    SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x15,0x04);
                    /*/Test*/
                    TotalCapacity = SiS_DRAMType[k][3] * buswidth;
                    SR13 = SiS_DRAMType[k][4];
                    if(buswidth == 4) SR14 = (TotalCapacity - 1) | 0x80;
                    if(buswidth == 2) SR14 = (TotalCapacity - 1) | 0x40;
                    if(buswidth == 1) SR14 = (TotalCapacity - 1) | 0x00;
                    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,SR13);
                    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,SR14);

                    Addr = FBAddr + (BankNumHigh) * 64 * 1024 + PhysicalAdrHigh;
                    *((USHORT *)(Addr)) = (USHORT)PhysicalAdrHigh;
                    Addr = FBAddr + (BankNumMid) * 64 * 1024 + PhysicalAdrHigh;
                    *((USHORT *)(Addr)) = (USHORT)BankNumMid;
                    Addr = FBAddr + (BankNumHigh) * 64 * 1024 + PhysicalAdrHalfPage;
                    *((USHORT *)(Addr)) = (USHORT)PhysicalAdrHalfPage;
                    Addr = FBAddr + (BankNumHigh) * 64 * 1024 + PhysicalAdrOtherPage;
                    *((USHORT *)(Addr)) = PhysicalAdrOtherPage;

                    /* Read data */
                    Addr = FBAddr + (BankNumHigh) * 64 * 1024 + PhysicalAdrHigh;
                    data = *((USHORT *)(Addr));
                    if(data == PhysicalAdrHigh) Done = 1;
                 }  /* if struct */
            }  /* for loop (k) */
         }  /* if struct */
      }  /* for loop (j) */
   }  /* for loop (i) */
}

USHORT
SiS_ChkBUSWidth_300(SiS_Private *SiS_Pr, ULONG FBAddress)
{
   PULONG  pVideoMemory;

   pVideoMemory = (PULONG)FBAddress;

   pVideoMemory[0] = 0x01234567L;
   pVideoMemory[1] = 0x456789ABL;
   pVideoMemory[2] = 0x89ABCDEFL;
   pVideoMemory[3] = 0xCDEF0123L;
   if (pVideoMemory[3]==0xCDEF0123L) {  /* Channel A 128bit */
     return(4);
   }
   if (pVideoMemory[1]==0x456789ABL) {  /* Channel B 64bit */
     return(2);
   }
   return(1);
}
#endif
/* ===============  SiS 300 dram sizing end    =============== */

/* ============  SiS 310/325 dram sizing begin  ============== */
#ifdef SIS315H

/* TW: Moved Get310DRAMType further down */

void
SiS_Delay15us(SiS_Private *SiS_Pr, ULONG ulMicrsoSec)
{
}

void
SiS_SDR_MRS(SiS_Private *SiS_Pr, )
{
   USHORT  data;

   data = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x16);
   data &= 0x3F;          		        /* SR16 D7=0, D6=0 */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,data);   	/* enable mode register set(MRS) low */
   SiS_Delay15us(SiS_Pr, 0x100);
   data |= 0x80;          		        /* SR16 D7=1, D6=0 */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,data);   	/* enable mode register set(MRS) high */
   SiS_Delay15us(SiS_Pr, 0x100);
}

void
SiS_DDR_MRS(SiS_Private *SiS_Pr)
{
   USHORT  data;

   /* SR16 <- 1F,DF,2F,AF */

   /* enable DLL of DDR SD/SGRAM , SR16 D4=1 */
   data=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x16);
   data &= 0x0F;
   data |= 0x10;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,data);

   if (!(SiS_Pr->SiS_SR15[1][SiS_Pr->SiS_RAMType] & 0x10))
     data &= 0x0F;

   /* SR16 D7=1,D6=1 */
   data |= 0xC0;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,data);
   
   /* SR16 D7=1,D6=0,D5=1,D4=0 */
   data &= 0x0F;
   data |= 0x20;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,data);
   if (!(SiS_Pr->SiS_SR15[1][SiS_Pr->SiS_RAMType] & 0x10))
     data &= 0x0F;

   /* SR16 D7=1 */
   data |= 0x80;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,data);
}

void
SiS_SetDRAMModeRegister(SiS_Private *SiS_Pr, UCHAR *ROMAddr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
    if (SiS_Get310DRAMType(ROMAddr,HwDeviceExtension) < 2)
        SiS_SDR_MRS(SiS_Pr);
    else
        /* SR16 <- 0F,CF,0F,8F */
        SiS_DDR_MRS(SiS_Pr);
}

void
SiS_DisableRefresh(SiS_Private *SiS_Pr)
{
   SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x17,0xF8);
   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x19,0x03);
}

void
SiS_EnableRefresh(SiS_Private *SiS_Pr, UCHAR *ROMAddr)
{
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x17,SiS_Pr->SiS_SR15[2][SiS_Pr->SiS_RAMType]);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x19,SiS_Pr->SiS_SR15[4][SiS_Pr->SiS_RAMType]);
}

void
SiS_DisableChannelInterleaving(SiS_Private *SiS_Pr, int index,
                               USHORT SiS_DDRDRAM_TYPE[][5])
{
   USHORT  data;

   data=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x15);
   data &= 0x1F;
   switch (SiS_DDRDRAM_TYPE[index][3])
   {
     case 64: data |= 0; 	break;
     case 32: data |= 0x20;	break;
     case 16: data |= 0x40;     break;
     case 4:  data |= 0x60;     break;
   }
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x15,data);
}

void
SiS_SetDRAMSizingType(SiS_Private *SiS_Pr, int index, USHORT DRAMTYPE_TABLE[][5])
{
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,DRAMTYPE_TABLE[index][4]);
   /* should delay 50 ns */
}

void
SiS_CheckBusWidth_310(SiS_Private *SiS_Pr, UCHAR *ROMAddress,ULONG FBAddress,
                      PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   USHORT  data;
   PULONG volatile pVideoMemory;

   pVideoMemory = (PULONG)FBAddress;
   if(SiS_Get310DRAMType(ROMAddress,HwDeviceExtension) < 2) {

     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,0x00);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,0x12);
     /* should delay */
     SiS_SDR_MRS(SiS_Pr);

     SiS_Pr->SiS_ChannelAB = 0;
     SiS_Pr->SiS_DataBusWidth = 128;
     pVideoMemory[0] = 0x01234567L;
     pVideoMemory[1] = 0x456789ABL;
     pVideoMemory[2] = 0x89ABCDEFL;
     pVideoMemory[3] = 0xCDEF0123L;
     pVideoMemory[4] = 0x55555555L;
     pVideoMemory[5] = 0x55555555L;
     pVideoMemory[6] = 0xFFFFFFFFL;
     pVideoMemory[7] = 0xFFFFFFFFL;
     if ((pVideoMemory[3]!=0xCDEF0123L) || (pVideoMemory[2] != 0x89ABCDEFL)) {
       /*Channel A 64Bit */
       SiS_Pr->SiS_DataBusWidth = 64;
       SiS_Pr->SiS_ChannelAB = 0;
       data=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,(USHORT)(data & 0xFD));
     }

     if ((pVideoMemory[1]!=0x456789ABL) || (pVideoMemory[0] != 0x01234567L)) {
       /*Channel B 64Bit */
       SiS_Pr->SiS_DataBusWidth = 64;
       SiS_Pr->SiS_ChannelAB = 1;
       data=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,(USHORT)((data&0xFD)|0x01));
     }
     return;

   } else {
     /* DDR Dual channel */
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x13,0x00);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,0x02); /* Channel A, 64bit */
     /* should delay */
     SiS_DDR_MRS(SiS_Pr);

     SiS_Pr->SiS_ChannelAB = 0;
     SiS_Pr->SiS_DataBusWidth = 64;
     pVideoMemory[0] = 0x01234567L;
     pVideoMemory[1] = 0x456789ABL;
     pVideoMemory[2] = 0x89ABCDEFL;
     pVideoMemory[3] = 0xCDEF0123L;
     pVideoMemory[4] = 0x55555555L;
     pVideoMemory[5] = 0x55555555L;
     pVideoMemory[6] = 0xAAAAAAAAL;
     pVideoMemory[7] = 0xAAAAAAAAL;

     if (pVideoMemory[1] == 0x456789ABL) {
       if (pVideoMemory[0] == 0x01234567L) {
         /* Channel A 64bit */
         return;
       }
     } else {
       if (pVideoMemory[0] == 0x01234567L) {
         /* Channel A 32bit */
         SiS_Pr->SiS_DataBusWidth = 32;
         SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,0x00);
         return;
       }
     }

     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,0x03); /* Channel B, 64bit */
     SiS_DDR_MRS(SiS_Pr);

     SiS_Pr->SiS_ChannelAB = 1;
     SiS_Pr->SiS_DataBusWidth = 64;
     pVideoMemory[0] = 0x01234567L;
     pVideoMemory[1] = 0x456789ABL;
     pVideoMemory[2] = 0x89ABCDEFL;
     pVideoMemory[3] = 0xCDEF0123L;
     pVideoMemory[4] = 0x55555555L;
     pVideoMemory[5] = 0x55555555L;
     pVideoMemory[6] = 0xAAAAAAAAL;
     pVideoMemory[7] = 0xAAAAAAAAL;
     if (pVideoMemory[1] == 0x456789ABL) {
       /* Channel B 64 */
       if (pVideoMemory[0] == 0x01234567L) {
         /* Channel B 64bit */
         return;
       } else {
         /* error */
       }
     } else {
       if (pVideoMemory[0] == 0x01234567L) {
         /* Channel B 32 */
         SiS_Pr->SiS_DataBusWidth = 32;
         SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,0x01);
       } else {
         /* error */
       }
     }
   }
}

int
SiS_SetRank(SiS_Private *SiS_Pr, int index,UCHAR RankNo,USHORT DRAMTYPE_TABLE[][5])
{
  USHORT  data;
  int RankSize;

  if ((RankNo==2)&&(DRAMTYPE_TABLE[index][0]==2))
         return 0;

  RankSize = DRAMTYPE_TABLE[index][3]/2 * SiS_Pr->SiS_DataBusWidth / 32;

  if (RankNo * RankSize <= 128) {
    data = 0;
    while((RankSize >>= 1) > 0) {
      data += 0x10;
    }
    data |= (RankNo - 1) << 2;
    data |= (SiS_Pr->SiS_DataBusWidth / 64) & 2;
    data |= SiS_Pr->SiS_ChannelAB;
    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,data);
    /* should delay */
    SiS_SDR_MRS(SiS_Pr);
    return 1;
  } else
    return 0;
}

int
SiS_SetDDRChannel(SiS_Private *SiS_Pr, int index,UCHAR ChannelNo,
                  USHORT DRAMTYPE_TABLE[][5])
{
  USHORT  data;
  int RankSize;

  RankSize = DRAMTYPE_TABLE[index][3]/2 * SiS_Pr->SiS_DataBusWidth / 32;
  /* RankSize = DRAMTYPE_TABLE[index][3]; */
  if (ChannelNo * RankSize <= 128) {
    data = 0;
    while((RankSize >>= 1) > 0) {
      data += 0x10;
    }
    if(ChannelNo == 2) data |= 0x0C;
    data |= (SiS_Pr->SiS_DataBusWidth / 32) & 2;
    data |= SiS_Pr->SiS_ChannelAB;
    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,data);
    /* should delay */
    SiS_DDR_MRS(SiS_Pr);
    return 1;
  } else
    return 0;
}

int
SiS_CheckColumn(SiS_Private *SiS_Pr, int index,USHORT DRAMTYPE_TABLE[][5],ULONG FBAddress)
{
  int i;
  ULONG Increment,Position;

  /*Increment = 1<<(DRAMTYPE_TABLE[index][2] + SiS_Pr->SiS_DataBusWidth / 64 + 1); */
  Increment = 1 << (10 + SiS_Pr->SiS_DataBusWidth / 64);

  for (i=0,Position=0;i<2;i++) {
         *((PULONG)(FBAddress + Position)) = Position;
         Position += Increment;
  }

  for (i=0,Position=0;i<2;i++) {
/*    if (FBAddress[Position]!=Position) */
         if((*(PULONG)(FBAddress + Position)) != Position)
                return 0;
         Position += Increment;
  }
  return 1;
}

int
SiS_CheckBanks(SiS_Private *SiS_Pr, int index,USHORT DRAMTYPE_TABLE[][5],ULONG FBAddress)
{
  int i;
  ULONG Increment,Position;
  Increment = 1 << (DRAMTYPE_TABLE[index][2] + SiS_Pr->SiS_DataBusWidth / 64 + 2);

  for (i=0,Position=0;i<4;i++) {
/*    FBAddress[Position]=Position; */
    *((PULONG)(FBAddress + Position)) = Position;
    Position += Increment;
  }

  for (i=0,Position=0;i<4;i++) {
/*    if (FBAddress[Position]!=Position) */
    if((*(PULONG)(FBAddress + Position)) != Position)
      return 0;
    Position += Increment;
  }
  return 1;
}

int
SiS_CheckRank(SiS_Private *SiS_Pr, int RankNo,int index,USHORT DRAMTYPE_TABLE[][5],ULONG FBAddress)
{
  int i;
  ULONG Increment,Position;
  Increment = 1<<(DRAMTYPE_TABLE[index][2] + DRAMTYPE_TABLE[index][1] +
                  DRAMTYPE_TABLE[index][0] + SiS_Pr->SiS_DataBusWidth / 64 + RankNo);

  for (i=0,Position=0;i<2;i++) {
/*    FBAddress[Position]=Position; */
    *((PULONG)(FBAddress+Position))=Position;
    /* *((PULONG)(FBAddress))=Position; */
    Position += Increment;
  }

  for (i=0,Position=0;i<2;i++) {
/*    if (FBAddress[Position]!=Position) */
         if ( (*(PULONG) (FBAddress + Position)) !=Position)
    /*if ( (*(PULONG) (FBAddress )) !=Position) */
      return 0;
    Position += Increment;
  }
  return 1;
}

int
SiS_CheckDDRRank(SiS_Private *SiS_Pr, int RankNo,int index,USHORT DRAMTYPE_TABLE[][5],ULONG FBAddress)
{
  ULONG Increment,Position;
  USHORT  data;

  Increment = 1<<(DRAMTYPE_TABLE[index][2] + DRAMTYPE_TABLE[index][1] +
                  DRAMTYPE_TABLE[index][0] + SiS_Pr->SiS_DataBusWidth / 64 + RankNo);

  Increment += Increment/2;

  Position =0;
  *((PULONG)(FBAddress+Position + 0)) = 0x01234567;
  *((PULONG)(FBAddress+Position + 1)) = 0x456789AB;
  *((PULONG)(FBAddress+Position + 2)) = 0x55555555;
  *((PULONG)(FBAddress+Position + 3)) = 0x55555555;
  *((PULONG)(FBAddress+Position + 4)) = 0xAAAAAAAA;
  *((PULONG)(FBAddress+Position + 5)) = 0xAAAAAAAA;

  if ( (*(PULONG) (FBAddress + 1)) == 0x456789AB)
    return 1;

  if ( (*(PULONG) (FBAddress + 0)) == 0x01234567)
    return 0;

  data=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
  data &= 0xF3;
  data |= 0x08;
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x14,data);
  data=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x15);
  data += 0x20;
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x15,data);

  return 1;
}

int
SiS_CheckRanks(SiS_Private *SiS_Pr, int RankNo,int index,USHORT DRAMTYPE_TABLE[][5],ULONG FBAddress)
{
  int r;

  for (r=RankNo;r>=1;r--) {
    if (!SiS_CheckRank(SiS_Pr, r, index, DRAMTYPE_TABLE, FBAddress))
      return 0;
  }
  if (!SiS_CheckBanks(SiS_Pr, index, DRAMTYPE_TABLE, FBAddress))
    return 0;

  if (!SiS_CheckColumn(SiS_Pr, index, DRAMTYPE_TABLE, FBAddress))
    return 0;

  return 1;
}

int
SiS_CheckDDRRanks(SiS_Private *SiS_Pr, int RankNo,int index,USHORT DRAMTYPE_TABLE[][5],
                  ULONG FBAddress)
{
  int r;

  for (r=RankNo;r>=1;r--) {
    if (!SiS_CheckDDRRank(SiS_Pr, r,index,DRAMTYPE_TABLE,FBAddress))
      return 0;
  }
  if (!SiS_CheckBanks(SiS_Pr, index,DRAMTYPE_TABLE,FBAddress))
    return 0;

  if (!SiS_CheckColumn(SiS_Pr, index,DRAMTYPE_TABLE,FBAddress))
    return 0;

  return 1;
}

int
SiS_SDRSizing(SiS_Private *SiS_Pr, ULONG FBAddress)
{
  int    i;
  UCHAR  j;

  for (i=0;i<13;i++) {
    SiS_SetDRAMSizingType(SiS_Pr, i, SiS_SDRDRAM_TYPE);
    for (j=2;j>0;j--) {
      if (!SiS_SetRank(SiS_Pr, i,(UCHAR) j, SiS_SDRDRAM_TYPE))
        continue;
      else {
        if (SiS_CheckRanks(SiS_Pr, j,i,SiS_SDRDRAM_TYPE, FBAddress))
          return 1;
      }
    }
  }
  return 0;
}

int
SiS_DDRSizing(SiS_Private *SiS_Pr, ULONG FBAddress)
{

  int    i;
  UCHAR  j;

  for (i=0; i<4; i++){
    SiS_SetDRAMSizingType(SiS_Pr, i, SiS_DDRDRAM_TYPE);
    SiS_DisableChannelInterleaving(SiS_Pr, i, SiS_DDRDRAM_TYPE);
    for (j=2; j>0; j--) {
      SiS_SetDDRChannel(SiS_Pr, i, j, SiS_DDRDRAM_TYPE);
      if (!SiS_SetRank(SiS_Pr, i, (UCHAR) j, SiS_DDRDRAM_TYPE))
        continue;
      else {
        if (SiS_CheckDDRRanks(SiS_Pr, j, i, SiS_DDRDRAM_TYPE, FBAddress))
          return 1;
      }
    }
  }
  return 0;
}

/*
 check if read cache pointer is correct
*/
void
SiS_VerifyMclk(SiS_Private *SiS_Pr, ULONG FBAddr)
{
   PUCHAR  pVideoMemory = (PUCHAR) FBAddr;
   UCHAR   i, j;
   USHORT  Temp,SR21;

   pVideoMemory[0] = 0xaa;  /* alan */
   pVideoMemory[16] = 0x55; /* note: PCI read cache is off */

   if((pVideoMemory[0] != 0xaa) || (pVideoMemory[16] != 0x55)) {
     for (i=0,j=16; i<2; i++,j+=16)  {
       SR21 = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x21);
       Temp = SR21 & 0xFB;           /* disable PCI post write buffer empty gating */
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x21,Temp);

       Temp = SiS_GetReg1(SiS_Pr->SiS_P3c4, 0x3C);
       Temp |= 0x01;                 /* MCLK reset */
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x3C,Temp);
       Temp = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x3C);
       Temp &= 0xFE;                 /* MCLK normal operation */
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x3C,Temp);
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x21,SR21);

       pVideoMemory[16+j] = j;
       if(pVideoMemory[16+j] == j) {
         pVideoMemory[j] = j;
         break;
       }
     }
   }
}

/* TW: Is this a 315E? */
int
Is315E(SiS_Private *SiS_Pr)
{
   USHORT  data;

   data = SiS_GetReg1(SiS_Pr->SiS_P3d4,0x5F);
   if(data & 0x10) return 1;
   else return 0;
}

/* TW: For 315 only */
void
SiS_SetDRAMSize_310(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   UCHAR  *ROMAddr  = HwDeviceExtension->pjVirtualRomBase;
   ULONG   FBAddr   = (ULONG)HwDeviceExtension->pjVideoMemoryAddress;
   USHORT  data;

#ifdef SIS301	    /* TW: SIS301 ??? */
   /*SiS_SetReg1(SiS_Pr->SiS_P3d4,0x30,0x40);   */
#endif
#ifdef SIS302       /* TW: SIS302 ??? */
   SiS_SetReg1(SiS_Pr->SiS_P3d4,0x30,0x4D);  /* alan,should change value */
   SiS_SetReg1(SiS_Pr->SiS_P3d4,0x31,0xc0);  /* alan,should change value */
   SiS_SetReg1(SiS_Pr->SiS_P3d4,0x34,0x3F);  /* alan,should change value */
#endif

   SiSSetMode(SiS_Pr, HwDeviceExtension, 0x2e);

   data = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x21);
   SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x21,0xDF);                 /* disable read cache */

   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x01,0x20);                  /* Turn OFF Display */

   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x16,0x0F);                  /* assume lowest speed DRAM */

   SiS_SetDRAMModeRegister(SiS_Pr, ROMAddr, HwDeviceExtension);
   SiS_DisableRefresh(SiS_Pr);
   SiS_CheckBusWidth_310(SiS_Pr, ROMAddr, FBAddr, HwDeviceExtension);

   SiS_VerifyMclk(SiS_Pr, FBAddr);

   if(SiS_Get310DRAMType(SiS_Pr, ROMAddr, HwDeviceExtension) < 2)
     SiS_SDRSizing(SiS_Pr, FBAddr);
   else
     SiS_DDRSizing(SiS_Pr, FBAddr);

   if(Is315E(SiS_Pr)) {
     data = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
     if((data & 0x0C) == 0x0C) { 	/* dual channel */
     	if((data & 0xF0) > 0x40)
     	  data = (data & 0x0F) | 0x40;
     } else { 				/* single channel */
     	if((data & 0xF0) > 0x50)
     	  data = (data & 0x0F) | 0x50;
     }
   }

   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x16,SiS_Pr->SiS_SR15[1][SiS_Pr->SiS_RAMType]);  /* restore SR16 */

   SiS_EnableRefresh(SiS_Pr, ROMAddr);
   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x21,0x20);      	/* enable read cache */
}
#endif

void
SiS_SetMemoryClock(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x28,SiS_Pr->SiS_MCLKData_0[SiS_Pr->SiS_RAMType].SR28);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x29,SiS_Pr->SiS_MCLKData_0[SiS_Pr->SiS_RAMType].SR29);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2A,SiS_Pr->SiS_MCLKData_0[SiS_Pr->SiS_RAMType].SR2A);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2E,SiS_Pr->SiS_ECLKData[SiS_Pr->SiS_RAMType].SR2E);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2F,SiS_Pr->SiS_ECLKData[SiS_Pr->SiS_RAMType].SR2F);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x30,SiS_Pr->SiS_ECLKData[SiS_Pr->SiS_RAMType].SR30);

#ifdef SIS315H
   if (Is315E(SiS_Pr)) {
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x28,0x3B); /* 143 */
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x29,0x22);
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2E,0x3B); /* 143 */
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2F,0x22);
   }
#endif
}

#endif /* ifdef LINUXBIOS */

#ifdef SIS315H
UCHAR
SiS_Get310DRAMType(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   UCHAR   data;

   if(*SiS_Pr->pSiS_SoftSetting & SoftDRAMType) {
     data = *SiS_Pr->pSiS_SoftSetting & 0x03;
   } else {
     if(HwDeviceExtension->jChipType > SIS_315PRO) {
        data = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x13) & 0x07;
     } else {	/* TW: 315 */
        data = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x3a) & 0x03;
     }
   }

   return data;
}
#endif

/* SiSInit END */

/* ----------------------------------------- */

void SiSRegInit(SiS_Private *SiS_Pr, USHORT BaseAddr)
{
   SiS_Pr->SiS_P3c4 = BaseAddr + 0x14;
   SiS_Pr->SiS_P3d4 = BaseAddr + 0x24;
   SiS_Pr->SiS_P3c0 = BaseAddr + 0x10;
   SiS_Pr->SiS_P3ce = BaseAddr + 0x1e;
   SiS_Pr->SiS_P3c2 = BaseAddr + 0x12;
   SiS_Pr->SiS_P3ca = BaseAddr + 0x1a;
   SiS_Pr->SiS_P3c6 = BaseAddr + 0x16;
   SiS_Pr->SiS_P3c7 = BaseAddr + 0x17;
   SiS_Pr->SiS_P3c8 = BaseAddr + 0x18;
   SiS_Pr->SiS_P3c9 = BaseAddr + 0x19;
   SiS_Pr->SiS_P3da = BaseAddr + 0x2A;
   SiS_Pr->SiS_Part1Port = BaseAddr + SIS_CRT2_PORT_04;   /* Digital video interface registers (LCD) */
   SiS_Pr->SiS_Part2Port = BaseAddr + SIS_CRT2_PORT_10;   /* 301 TV Encoder registers */
   SiS_Pr->SiS_Part3Port = BaseAddr + SIS_CRT2_PORT_12;   /* 301 Macrovision registers */
   SiS_Pr->SiS_Part4Port = BaseAddr + SIS_CRT2_PORT_14;   /* 301 VGA2 (and LCD) registers */
   SiS_Pr->SiS_Part5Port = BaseAddr + SIS_CRT2_PORT_14+2; /* 301 palette address port registers */
   SiS_Pr->SiS_DDC_Port = BaseAddr + 0x14;                /* DDC Port ( = P3C4, SR11/0A) */
}

void
SiSInitPCIetc(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
/* #ifdef LINUX_XF86 */
   if ((HwDeviceExtension->jChipType == SIS_540)||
       (HwDeviceExtension->jChipType == SIS_630)||
       (HwDeviceExtension->jChipType == SIS_730)||
       (HwDeviceExtension->jChipType == SIS_300)) {
       /* TW: Set - PCI LINEAR ADDRESSING ENABLE (0x80)
		  - PCI IO ENABLE  (0x20)
		  - MMIO ENABLE (0x1)
  	*/
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x20,0xa1);
       /* TW: Enable 2D (0x42) & 3D accelerator (0x18) */
       SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x1E,0xFF,0x5A);
   }
   if((HwDeviceExtension->jChipType == SIS_315H)||
      (HwDeviceExtension->jChipType == SIS_315PRO)||
      (HwDeviceExtension->jChipType == SIS_550)||
      (HwDeviceExtension->jChipType == SIS_640)||
      (HwDeviceExtension->jChipType == SIS_740)||
      (HwDeviceExtension->jChipType == SIS_650)) {
      /* TW: This seems to be done the same way on these chipsets */
      SiS_SetReg1(SiS_Pr->SiS_P3c4,0x20,0xa1);
      SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x1E,0xFF,0x5A);
   }
/* #endif */
}

void
SiSSetLVDSetc(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,USHORT ModeNo)
{
   ULONG   temp;

   SiS_Pr->SiS_IF_DEF_LVDS = 0;
   SiS_Pr->SiS_IF_DEF_TRUMPION = 0;
   SiS_Pr->SiS_IF_DEF_CH70xx = 0;
   SiS_Pr->SiS_IF_DEF_HiVision = 0;
   SiS_Pr->SiS_IF_DEF_DSTN = 0;
   SiS_Pr->SiS_IF_DEF_FSTN = 0;

   SiS_Pr->SiS_ChrontelInit = 0;

   if((ModeNo == 0x5a) || (ModeNo == 0x5b)) {
   	SiS_Pr->SiS_IF_DEF_DSTN = 1;   /* for 550 dstn */
   	SiS_Pr->SiS_IF_DEF_FSTN = 1;   /* for fstn */
   }

#ifdef SIS300
   if((HwDeviceExtension->jChipType == SIS_540) ||
      (HwDeviceExtension->jChipType == SIS_630) ||
      (HwDeviceExtension->jChipType == SIS_730))
    {
        /* TW: Check for SiS30x first */
        temp = SiS_GetReg1(SiS_Pr->SiS_Part4Port,0x00);
	if((temp == 1) || (temp == 2)) return;
      	temp = SiS_GetReg1(SiS_Pr->SiS_P3d4,0x37);
      	temp = (temp & 0x0E) >> 1;
      	if((temp >= 2) && (temp <= 5)) SiS_Pr->SiS_IF_DEF_LVDS = 1;
      	if(temp == 3)   SiS_Pr->SiS_IF_DEF_TRUMPION = 1;
      	if((temp == 4) || (temp == 5)) {
		/* TW: Save power status (and error check) */
		SiS_Pr->SiS_Backup70xx = SiS_GetCH700x(SiS_Pr, 0x0e);
		SiS_Pr->SiS_IF_DEF_CH70xx = 1;
        }
   }
#endif
#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650))
    {
        /* TW: CR37 is different on 310/325 series */
        if (SiS_Pr->SiS_IF_DEF_FSTN)                       /* fstn: set CR37=0x04 */
             SiS_SetReg1(SiS_Pr->SiS_P3d4,0x37,0x04);      /* (fake LVDS bridge) */

	temp=SiS_GetReg1(SiS_Pr->SiS_P3d4,0x37);
      	temp = (temp & 0x0E) >> 1;
      	if((temp >= 2) && (temp <= 3)) SiS_Pr->SiS_IF_DEF_LVDS = 1;
      	if(temp == 3)  {
			SiS_Pr->SiS_IF_DEF_CH70xx = 2;
        }
	/* SiS_Pr->SiS_IF_DEF_HiVision = 1; */
    }
#endif
}

void
SiSInitPtr(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
#ifdef SIS315H
   if((HwDeviceExtension->jChipType == SIS_315H) ||
      (HwDeviceExtension->jChipType == SIS_315PRO) ||
      (HwDeviceExtension->jChipType == SIS_550) ||
      (HwDeviceExtension->jChipType == SIS_640) ||
      (HwDeviceExtension->jChipType == SIS_740) ||
      (HwDeviceExtension->jChipType == SIS_650))
     InitTo310Pointer(SiS_Pr, HwDeviceExtension);
#endif

#ifdef SIS300
   if ((HwDeviceExtension->jChipType == SIS_540) ||
       (HwDeviceExtension->jChipType == SIS_630) ||
       (HwDeviceExtension->jChipType == SIS_730) ||
       (HwDeviceExtension->jChipType == SIS_300))
     InitTo300Pointer(SiS_Pr, HwDeviceExtension);
#endif
}

void
SiSDetermineROMUsage(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension, UCHAR *ROMAddr)
{
   if((ROMAddr) && (HwDeviceExtension->UseROM)) {
     if((ROMAddr[0x00] != 0x55) || (ROMAddr[0x01] != 0xAA)) {
        SiS_Pr->SiS_UseROM = FALSE;
     } else if(HwDeviceExtension->jChipType < SIS_315H) {
        /* TW: We don't use the ROM image if BIOS version < 2.0.0 as
         *     such old BIOSes don't have the needed data at the
	 *     expected locations
	 */
        if(ROMAddr[0x06] < '2')  SiS_Pr->SiS_UseROM = FALSE;
	else                     SiS_Pr->SiS_UseROM = TRUE;
     } else {
        /* TW: TODO: Check this for 310/325 series */
	SiS_Pr->SiS_UseROM = TRUE;
     }
   } else SiS_Pr->SiS_UseROM = FALSE;

}

/*
 	=========================================
 	======== SiS SetMode Functions ==========
 	=========================================
*/
#ifdef LINUX_XF86
/* TW: This is used for non-Dual-Head mode from X */
BOOLEAN
SiSBIOSSetMode(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension, ScrnInfoPtr pScrn,
               DisplayModePtr mode)
{
   UShort  ModeNo=0;

   ModeNo = SiS_CalcModeIndex(pScrn, mode);
   if(!ModeNo) return FALSE;

   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting mode 0x%x\n", ModeNo);

   return(SiSSetMode(SiS_Pr, HwDeviceExtension, pScrn, ModeNo, TRUE));
}

#ifdef SISDUALHEAD
/* TW: Set CRT1 mode (used for dual head) */
BOOLEAN
SiSBIOSSetModeCRT1(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension, ScrnInfoPtr pScrn,
               DisplayModePtr mode)
{
   ULONG   temp;
   USHORT  ModeIdIndex;
   UCHAR  *ROMAddr  = HwDeviceExtension->pjVirtualRomBase;
   USHORT  BaseAddr = (USHORT)HwDeviceExtension->ulIOAddress;
   SISPtr  pSiS = SISPTR(pScrn);
   SISEntPtr pSiSEnt = pSiS->entityPrivate;

   UShort  ModeNo=0;

   ModeNo = SiS_CalcModeIndex(pScrn, mode);
   if(!ModeNo) return FALSE;

   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting mode 0x%x on CRT1\n", ModeNo);

   SiSInitPtr(SiS_Pr, HwDeviceExtension);

   SiSRegInit(SiS_Pr, BaseAddr);

   SiS_Pr->SiS_VGAINFO = SiS_GetSetMMIOReg(pScrn, 0x489, 0xff);

   SiSInitPCIetc(SiS_Pr, HwDeviceExtension);

   SiSSetLVDSetc(SiS_Pr, HwDeviceExtension, ModeNo);

   SiSDetermineROMUsage(SiS_Pr, HwDeviceExtension, ROMAddr);

   /* TW: We don't clear the buffer under X */
   SiS_Pr->SiS_flag_clearbuffer=0;

   /* 1.Openkey */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x05,0x86);

   SiS_UnLockCRT2(SiS_Pr, HwDeviceExtension, BaseAddr);

   /* 2.Get ModeID Table  */
   temp = SiS_SearchModeID(SiS_Pr, ROMAddr,&ModeNo,&ModeIdIndex);
   if(temp == 0)  return(0);

   /* TW: Determine VBType (301,301B,301LV,302B,302LV) */
   SiS_GetVBType(SiS_Pr, BaseAddr,HwDeviceExtension);

   /* TW: Get VB information (connectors, connected devices) */
   SiS_GetVBInfo(SiS_Pr, BaseAddr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);
   SiS_SetHiVision(SiS_Pr, BaseAddr,HwDeviceExtension);
   SiS_GetLCDResInfo(SiS_Pr, ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);

   /* TW: I am not sure the flag's name is correct */
   if(HwDeviceExtension->jChipType >= SIS_315H) {
      if(SiS_GetReg1(SiS_Pr->SiS_P3c4,0x17) & 0x08)  {
          if(ModeNo != 0x10)  SiS_Pr->SiS_SetFlag |= CRT2IsVGA;
      }
   }

   /* TW: Set mode on CRT1 */
   SiS_SetCRT1Group(SiS_Pr, ROMAddr,HwDeviceExtension,ModeNo,ModeIdIndex,BaseAddr);

   pSiSEnt->CRT1ModeNo = ModeNo;
   pSiSEnt->CRT1DMode = mode;

   /* TW: SetPitch: Adapt to virtual size & position */
   if(ModeNo > 0x13) {
      SiS_SetPitchCRT1(SiS_Pr, pScrn, BaseAddr);
   }

   /* We have to reset CRT2 if changing mode on CRT1 */
   if(pSiSEnt->CRT2ModeNo != -1) {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, "(Re-)Setting mode 0x%x on CRT2\n",
				pSiSEnt->CRT2ModeNo);
	SiSBIOSSetModeCRT2(SiS_Pr, HwDeviceExtension, pSiSEnt->pScrn_1,
				pSiSEnt->CRT2DMode);
   }

   if((HwDeviceExtension->jChipType > SIS_315PRO) && (SiS_Pr->SiS_IF_DEF_LVDS == 0)) {
      /* TW: *** For 650 only! *** */
      SiS_HandleCRT1(SiS_Pr);
   }

   SiS_DisplayOn(SiS_Pr);
   SiS_SetReg3(SiS_Pr->SiS_P3c6,0xFF);

   if((HwDeviceExtension->jChipType >= SIS_315H) && (SiS_Pr->SiS_IF_DEF_LVDS == 0)) {
      if(SiS_Pr->SiS_LCDResInfo == SiS_Pr->SiS_Panel1400x1050) {
          SiS_Handle301B_1400x1050(SiS_Pr, ModeNo);
      }
   }

   /* Backup/Set ModeNo in MMIO */
   SiS_GetSetModeID(pScrn,ModeNo);

   return TRUE;
}

/* TW: Set CRT2 mode (used for dual head) */
BOOLEAN
SiSBIOSSetModeCRT2(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension, ScrnInfoPtr pScrn,
               DisplayModePtr mode)
{
   ULONG   temp;
   USHORT  ModeIdIndex;
   UCHAR  *ROMAddr  = HwDeviceExtension->pjVirtualRomBase;
   USHORT  BaseAddr = (USHORT)HwDeviceExtension->ulIOAddress;
   UShort  ModeNo   = 0;
   SISPtr  pSiS     = SISPTR(pScrn);
   SISEntPtr pSiSEnt = pSiS->entityPrivate;

   ModeNo = SiS_CalcModeIndex(pScrn, mode);
   if(!ModeNo) return FALSE;

   SiSInitPtr(SiS_Pr, HwDeviceExtension);

   SiSRegInit(SiS_Pr, BaseAddr);

   SiS_Pr->SiS_VGAINFO = SiS_GetSetMMIOReg(pScrn, 0x489, 0xff);

   SiSInitPCIetc(SiS_Pr, HwDeviceExtension);

   SiSSetLVDSetc(SiS_Pr, HwDeviceExtension, ModeNo);

   SiSDetermineROMUsage(SiS_Pr, HwDeviceExtension, ROMAddr);

   /* TW: We don't clear the buffer under X */
   SiS_Pr->SiS_flag_clearbuffer=0;

   /* TW: Save ModeNo so we can set it from within SetMode for CRT1 */
   pSiSEnt->CRT2ModeNo = ModeNo;
   pSiSEnt->CRT2DMode = mode;

   /* TW: We can't set CRT2 mode before CRT1 mode is set */
   if(pSiSEnt->CRT1ModeNo == -1) {
   	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		"Setting CRT2 mode delayed until after setting CRT1 mode\n");
   	return TRUE;
   }

   xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Setting mode 0x%x on CRT2\n", ModeNo);

   /* 1.Openkey */
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x05,0x86);

   SiS_UnLockCRT2(SiS_Pr, HwDeviceExtension, BaseAddr);

   /* 2.Get ModeID */
   temp = SiS_SearchModeID(SiS_Pr, ROMAddr,&ModeNo,&ModeIdIndex);
   if(temp == 0)  return(0);

   /* TW: Determine VBType (301,301B,301LV,302B,302LV) */
   SiS_GetVBType(SiS_Pr, BaseAddr,HwDeviceExtension);

   /* TW: Get VB information (connectors, connected devices) */
   SiS_GetVBInfo(SiS_Pr, BaseAddr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);
   SiS_SetHiVision(SiS_Pr, BaseAddr,HwDeviceExtension);
   SiS_GetLCDResInfo(SiS_Pr, ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);

   if(HwDeviceExtension->jChipType >= SIS_315H) {
      if(SiS_GetReg1(SiS_Pr->SiS_P3c4,0x17) & 0x08)  {
          /* TW: I am not sure the flag's name is correct */
          if(ModeNo != 0x10)  SiS_Pr->SiS_SetFlag |= CRT2IsVGA;
      }
   }

   /* Set mode on CRT2 */
   switch (HwDeviceExtension->ujVBChipID) {
     case VB_CHIP_301:
     case VB_CHIP_301B:
     case VB_CHIP_301LV:
     case VB_CHIP_301LVX:
     case VB_CHIP_302:
     case VB_CHIP_302B:
     case VB_CHIP_302LV:
     case VB_CHIP_302LVX:
        SiS_SetCRT2Group301(SiS_Pr, BaseAddr,ROMAddr,ModeNo,HwDeviceExtension);
        break;
     case VB_CHIP_303:
        break;
     case VB_CHIP_UNKNOWN:
        if (SiS_Pr->SiS_IF_DEF_LVDS == 1 || SiS_Pr->SiS_IF_DEF_CH70xx == 1 ||
	                                               SiS_Pr->SiS_IF_DEF_TRUMPION != 0)
             	SiS_SetCRT2Group301(SiS_Pr,BaseAddr,ROMAddr,ModeNo,HwDeviceExtension);
        break;
   }

   SiS_DisplayOn(SiS_Pr);
   SiS_SetReg3(SiS_Pr->SiS_P3c6,0xFF);

   if((HwDeviceExtension->jChipType >= SIS_315H) && (SiS_Pr->SiS_IF_DEF_LVDS == 0)) {
      if(SiS_Pr->SiS_LCDResInfo == SiS_Pr->SiS_Panel1400x1050) {
          SiS_Handle301B_1400x1050(SiS_Pr, ModeNo);
      }
   }

   /* TW: SetPitch: Adapt to virtual size & position */
   if(ModeNo > 0x13) {
       SiS_SetPitchCRT2(SiS_Pr, pScrn, BaseAddr);
   }

   return TRUE;
}
#endif /* Dualhead */
#endif /* Linux_XF86 */

#ifdef LINUX_XF86
/* TW: We need pScrn for setting the pitch correctly */
BOOLEAN
SiSSetMode(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,ScrnInfoPtr pScrn,USHORT ModeNo, BOOLEAN dosetpitch)
#else
BOOLEAN
SiSSetMode(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,USHORT ModeNo)
#endif
{
   ULONG   temp;
   USHORT  ModeIdIndex,KeepLockReg;
   UCHAR  *ROMAddr  = HwDeviceExtension->pjVirtualRomBase;
   USHORT  BaseAddr = (USHORT)HwDeviceExtension->ulIOAddress;

   SiSInitPtr(SiS_Pr, HwDeviceExtension);

   SiSRegInit(SiS_Pr, BaseAddr);

#ifdef LINUX_XF86
   if(pScrn) SiS_Pr->SiS_VGAINFO = SiS_GetSetMMIOReg(pScrn, 0x489, 0xff);
   else
#endif
         SiS_Pr->SiS_VGAINFO = 0x11;

   SiSInitPCIetc(SiS_Pr, HwDeviceExtension);

   SiSSetLVDSetc(SiS_Pr, HwDeviceExtension, ModeNo);

   SiSDetermineROMUsage(SiS_Pr, HwDeviceExtension, ROMAddr);

   /* TW: Shift the clear-buffer-bit away */
   ModeNo = ((ModeNo & 0x80) << 8) | (ModeNo & 0x7f);

#ifdef LINUX_XF86
   /* TW: We never clear the buffer in X */
   ModeNo |= 0x8000;
#endif

   if(ModeNo & 0x8000) {
     	ModeNo &= 0x007F;
     	SiS_Pr->SiS_flag_clearbuffer = 0;
   } else {
     	SiS_Pr->SiS_flag_clearbuffer = 1;
   }

   /* 1.Openkey */
   KeepLockReg = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x05);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x05,0x86);

   SiS_UnLockCRT2(SiS_Pr, HwDeviceExtension, BaseAddr);

   /* 2.Get ModeID Table  */
   temp = SiS_SearchModeID(SiS_Pr,ROMAddr,&ModeNo,&ModeIdIndex);
   if(temp == 0) return(0);

   /* TW: Determine VBType (301,301B,301LV,302B,302LV) */
   SiS_GetVBType(SiS_Pr,BaseAddr,HwDeviceExtension);

   /* TW: Init/restore some VB registers */
   if(HwDeviceExtension->jChipType >= SIS_315H) {
      if(SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV) {
         if(ROMAddr && SiS_Pr->SiS_UseROM) {
           temp = ROMAddr[VB310Data_1_2_Offset];
	   temp |= 0x40;
           SiS_SetReg1(SiS_Pr->SiS_Part1Port,0x02,temp);
	   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x32,0x10);
         }
      }
   }

   /* TW: Get VB information (connectors, connected devices) */
   SiS_GetVBInfo(SiS_Pr,BaseAddr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);
   SiS_SetHiVision(SiS_Pr,BaseAddr,HwDeviceExtension);
   SiS_GetLCDResInfo(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);

   /* 3. Check memory size */
   temp = SiS_CheckMemorySize(SiS_Pr,ROMAddr,HwDeviceExtension,ModeNo,ModeIdIndex);
   if(!temp) return(0);

   if(HwDeviceExtension->jChipType >= SIS_315H) {
      if(SiS_GetReg1(SiS_Pr->SiS_P3c4,0x17) & 0x08)  {
          /* TW: I am not sure the flag's name is correct */
          if(ModeNo != 0x10)  SiS_Pr->SiS_SetFlag |= CRT2IsVGA;
      }
   }

   /* TW: Set mode on CRT1 */
   if(SiS_Pr->SiS_VBInfo & (SetSimuScanMode | SetCRT2ToLCDA)) {
   	SiS_SetCRT1Group(SiS_Pr,ROMAddr,HwDeviceExtension,ModeNo,ModeIdIndex,BaseAddr);
   } else {
     if(!(SiS_Pr->SiS_VBInfo & SwitchToCRT2)) {
       	SiS_SetCRT1Group(SiS_Pr,ROMAddr,HwDeviceExtension,ModeNo,ModeIdIndex,BaseAddr);
     }
   }

   /* TW: Set mode on CRT2 */
   if(SiS_Pr->SiS_VBInfo & (SetSimuScanMode | SwitchToCRT2 | SetCRT2ToLCDA)) {
     switch (HwDeviceExtension->ujVBChipID) {
     case VB_CHIP_301:
     case VB_CHIP_301B:
     case VB_CHIP_301LV:
     case VB_CHIP_301LVX:
     case VB_CHIP_302:
     case VB_CHIP_302B:
     case VB_CHIP_302LV:
     case VB_CHIP_302LVX:
        SiS_SetCRT2Group301(SiS_Pr,BaseAddr,ROMAddr,ModeNo,HwDeviceExtension);
        break;
     case VB_CHIP_303:
        break;
     case VB_CHIP_UNKNOWN:
	if(SiS_Pr->SiS_IF_DEF_LVDS == 1   ||
	   SiS_Pr->SiS_IF_DEF_CH70xx != 0 ||
	   SiS_Pr->SiS_IF_DEF_TRUMPION != 0)
             	SiS_SetCRT2Group301(SiS_Pr,BaseAddr,ROMAddr,ModeNo,HwDeviceExtension);
        break;
     }
   }


   if((HwDeviceExtension->jChipType > SIS_315PRO) && (SiS_Pr->SiS_IF_DEF_LVDS == 0)) {
      /* TW: For 650 only! */
      SiS_HandleCRT1(SiS_Pr);
   }

   SiS_DisplayOn(SiS_Pr);
   SiS_SetReg3(SiS_Pr->SiS_P3c6,0xFF);

   if((HwDeviceExtension->jChipType >= SIS_315H) && (SiS_Pr->SiS_IF_DEF_LVDS == 0)) {
      if(SiS_Pr->SiS_LCDResInfo == SiS_Pr->SiS_Panel1400x1050) {
          SiS_Handle301B_1400x1050(SiS_Pr, ModeNo);
      }
   }

#ifdef LINUX_XF86
   if(pScrn) {
      /* TW: SetPitch: Adapt to virtual size & position */
      if((ModeNo > 0x13) && (dosetpitch)) {
         SiS_SetPitch(SiS_Pr, pScrn, BaseAddr);
      }

      /* Backup/Set ModeNo in MMIO */
      SiS_GetSetModeID(pScrn, ModeNo);
   }
#endif

#ifndef LINUX_XF86      /* TW: We never lock registers in XF86 */
   if(KeepLockReg == 0xA1) SiS_SetReg1(SiS_Pr->SiS_P3c4,0x05,0x86);
   else SiS_SetReg1(SiS_Pr->SiS_P3c4,0x05,0x00);
#endif

   return TRUE;
}

void
SetEnableDstn(SiS_Private *SiS_Pr)	/* TW: Called from sis_main.c */
{
   /* For 550 dstn */
   SiS_Pr->SiS_IF_DEF_DSTN = 1;
}

void
SiS_HandleCRT1(SiS_Private *SiS_Pr)
{
  /* TW: Do this on 650 only! */

  /* TW: No, we don't do this at all. There is a new
   * CRT1-is-connected-at-boot-time logic in the 650, which
   * confuses our own. So just clear the bit and skip the rest.
   */

  SiS_SetRegAND(SiS_Pr->SiS_P3d4,0x63,0xbf);

}

void
SiS_Handle301B_1400x1050(SiS_Private *SiS_Pr, USHORT ModeNo)
{
  if(SiS_GetReg1(SiS_Pr->SiS_P3d4,0x30) & SetCRT2ToLCD) {
     if(ModeNo <= 0x13) {
        if(SiS_GetReg1(SiS_Pr->SiS_P3d4,0x31) & (SetNotSimuMode >> 8)) {
	   SiS_SetRegAND(SiS_Pr->SiS_P3d4,0x38,0xFC);
	}
     }
  }
}

void
SiS_SetCRT1Group(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                 USHORT ModeNo,USHORT ModeIdIndex,USHORT BaseAddr)
{
  USHORT  StandTableIndex,RefreshRateTableIndex;

  SiS_Pr->SiS_CRT1Mode = ModeNo;
  StandTableIndex = SiS_GetModePtr(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex);
  if(SiS_LowModeStuff(SiS_Pr,ModeNo,HwDeviceExtension)) {
    if(SiS_Pr->SiS_VBInfo & (SetSimuScanMode | SwitchToCRT2)) {
       SiS_DisableBridge(SiS_Pr,HwDeviceExtension,BaseAddr);
    }
  }

  SiS_SetSeqRegs(SiS_Pr,ROMAddr,StandTableIndex);
  SiS_SetMiscRegs(SiS_Pr,ROMAddr,StandTableIndex);
  SiS_SetCRTCRegs(SiS_Pr,ROMAddr,HwDeviceExtension,StandTableIndex);
  SiS_SetATTRegs(SiS_Pr,ROMAddr,StandTableIndex,ModeNo,HwDeviceExtension);
  SiS_SetGRCRegs(SiS_Pr,ROMAddr,StandTableIndex);
  SiS_ClearExt1Regs(SiS_Pr,HwDeviceExtension);
  SiS_ResetCRT1VCLK(SiS_Pr,ROMAddr,HwDeviceExtension);

  SiS_Pr->SiS_SelectCRT2Rate = 0;
  SiS_Pr->SiS_SetFlag &= (~ProgrammingCRT2);

#ifdef LINUX_XF86
  xf86DrvMsg(0, X_PROBED, "(init: VBType=0x%04x, VBInfo=0x%04x)\n",
                    SiS_Pr->SiS_VBType, SiS_Pr->SiS_VBInfo);
#endif

  if(SiS_Pr->SiS_VBInfo & SetSimuScanMode) {
     if(SiS_Pr->SiS_VBInfo & SetInSlaveMode) {
        SiS_Pr->SiS_SetFlag |= ProgrammingCRT2;
     }
  }

  if(SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA) {
	SiS_Pr->SiS_SetFlag |= ProgrammingCRT2;
  }

  RefreshRateTableIndex = SiS_GetRatePtrCRT2(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);

  if(!(SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA)) {
	SiS_Pr->SiS_SetFlag &= (~ProgrammingCRT2);
  }

  if (RefreshRateTableIndex != 0xFFFF) {
    	SiS_SetSync(SiS_Pr,ROMAddr,RefreshRateTableIndex);
    	SiS_SetCRT1CRTC(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,RefreshRateTableIndex,HwDeviceExtension);
    	SiS_SetCRT1Offset(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,RefreshRateTableIndex,HwDeviceExtension);
    	SiS_SetCRT1VCLK(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension,RefreshRateTableIndex);
  }

#ifdef SIS300
  if(HwDeviceExtension->jChipType == SIS_300){
     	SiS_SetCRT1FIFO_300(SiS_Pr,ROMAddr,ModeNo,HwDeviceExtension,RefreshRateTableIndex);
  }
  if((HwDeviceExtension->jChipType == SIS_630)||
     (HwDeviceExtension->jChipType == SIS_730)||
     (HwDeviceExtension->jChipType == SIS_540)) {
     	SiS_SetCRT1FIFO_630(SiS_Pr,ROMAddr,ModeNo,HwDeviceExtension,RefreshRateTableIndex);
  }
#endif
#ifdef SIS315H
  if(HwDeviceExtension->jChipType >= SIS_315H) {
     	SiS_SetCRT1FIFO_310(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,HwDeviceExtension);
  }
#endif

  SiS_SetCRT1ModeRegs(SiS_Pr,ROMAddr,HwDeviceExtension,ModeNo,ModeIdIndex,RefreshRateTableIndex);

  SiS_LoadDAC(SiS_Pr,HwDeviceExtension,ROMAddr,ModeNo,ModeIdIndex);

#ifndef LINUX_XF86
  if(SiS_Pr->SiS_flag_clearbuffer) {
        SiS_ClearBuffer(SiS_Pr,HwDeviceExtension,ModeNo);
  }
#endif

  if(!(SiS_Pr->SiS_VBInfo & (SetSimuScanMode | SwitchToCRT2 | SetCRT2ToLCDA))) {
        SiS_LongWait(SiS_Pr);
        SiS_DisplayOn(SiS_Pr);
  }
}

#ifdef LINUX_XF86
void
SiS_SetPitch(SiS_Private *SiS_Pr, ScrnInfoPtr pScrn, UShort BaseAddr)
{
   SISPtr pSiS = SISPTR(pScrn);

   /* TW: We need to set pitch for CRT1 if bridge is in SlaveMode, too */
   if( (pSiS->VBFlags & DISPTYPE_DISP1) ||
       ( (pSiS->VBFlags & VB_VIDEOBRIDGE) &&
         ( ((pSiS->VGAEngine == SIS_300_VGA) && (SiS_GetReg1(SiS_Pr->SiS_Part1Port,0x00) & 0xa0) == 0x20) ||
           ((pSiS->VGAEngine == SIS_315_VGA) && (SiS_GetReg1(SiS_Pr->SiS_Part1Port,0x00) & 0x50) == 0x10) ) ) ) {
   	SiS_SetPitchCRT1(SiS_Pr, pScrn, BaseAddr);
   }
   if (pSiS->VBFlags & DISPTYPE_DISP2) {
   	SiS_SetPitchCRT2(SiS_Pr, pScrn, BaseAddr);
   }
}

void
SiS_SetPitchCRT1(SiS_Private *SiS_Pr, ScrnInfoPtr pScrn, UShort BaseAddr)
{
    SISPtr pSiS = SISPTR(pScrn);
    ULong  HDisplay,temp;

    HDisplay = pSiS->scrnPitch / 8;
    SiS_SetReg1(SiS_Pr->SiS_P3d4, 0x13, (HDisplay & 0xFF));
    temp = (SiS_GetReg1(SiS_Pr->SiS_P3c4, 0x0E) & 0xF0) | (HDisplay>>8);
    SiS_SetReg1(SiS_Pr->SiS_P3c4, 0x0E, temp);
}

void
SiS_SetPitchCRT2(SiS_Private *SiS_Pr, ScrnInfoPtr pScrn, UShort BaseAddr)
{
    SISPtr pSiS = SISPTR(pScrn);
    ULong  HDisplay,temp;

    HDisplay = pSiS->scrnPitch / 8;

    /* Unlock CRT2 */
    if (pSiS->VGAEngine == SIS_315_VGA)
        SiS_SetRegANDOR(SiS_Pr->SiS_Part1Port,0x2F, 0xFF, 0x01);
    else
        SiS_SetRegANDOR(SiS_Pr->SiS_Part1Port,0x24, 0xFF, 0x01);

    SiS_SetReg1(SiS_Pr->SiS_Part1Port,0x07, (HDisplay & 0xFF));
    temp = (SiS_GetReg1(SiS_Pr->SiS_Part1Port,0x09) & 0xF0) | ((HDisplay >> 8) & 0xFF);
    SiS_SetReg1(SiS_Pr->SiS_Part1Port,0x09, temp);
}
#endif

/* TW: Checked against 650/301 and 630/301B BIOS */
/* TW: Re-written for 650/301LVx 1.10.6s BIOS */
void
SiS_GetVBType(SiS_Private *SiS_Pr, USHORT BaseAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  USHORT flag=0, rev=0, nolcd=0;

  SiS_Pr->SiS_VBType = 0;

  if(SiS_Pr->SiS_IF_DEF_LVDS == 1) return;

  flag = SiS_GetReg1(SiS_Pr->SiS_Part4Port,0x00);

  /* TW: Illegal values not welcome... */
  if(flag > 10) return;

  rev = SiS_GetReg1(SiS_Pr->SiS_Part4Port,0x01);

  if (flag >= 2) {
        SiS_Pr->SiS_VBType = VB_SIS302B;
  } else if (flag == 1) {
        SiS_Pr->SiS_VBType = VB_SIS301;
        if(rev >= 0xB0) {
            	SiS_Pr->SiS_VBType = VB_SIS301B;
		if(HwDeviceExtension->jChipType >= SIS_315H) {
    		    nolcd = SiS_GetReg1(SiS_Pr->SiS_Part4Port,0x23);
                    if(!(nolcd & 0x02))
       	                SiS_Pr->SiS_VBType |= VB_NoLCD;
		}
        }
  }
  if(SiS_Pr->SiS_VBType & (VB_SIS301B | VB_SIS302B)) {
        if(rev >= 0xD0) {
	        SiS_Pr->SiS_VBType &= ~(VB_SIS301B | VB_SIS302B);
          	SiS_Pr->SiS_VBType |= VB_SIS30xLV;
		SiS_Pr->SiS_VBType &= ~(VB_NoLCD);
		if(rev >= 0xE0) {
		    SiS_Pr->SiS_VBType &= ~(VB_SIS30xLV);
		    SiS_Pr->SiS_VBType |= VB_SIS30xNEW;
		}
        }
  }
}

/* TW: Checked against 650/301LVx 1.10.6s */
BOOLEAN
SiS_SearchModeID(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT *ModeNo,USHORT *ModeIdIndex)
{
   UCHAR VGAINFO = SiS_Pr->SiS_VGAINFO;

   if(*ModeNo <= 0x13) {

      if((*ModeNo) <= 5) (*ModeNo) |= 1;

      for (*ModeIdIndex=0;;(*ModeIdIndex)++) {
         if (SiS_Pr->SiS_SModeIDTable[*ModeIdIndex].St_ModeID == (*ModeNo)) break;
         if (SiS_Pr->SiS_SModeIDTable[*ModeIdIndex].St_ModeID == 0xFF)   return FALSE;
      }

      if(*ModeNo == 0x07) {
          if(VGAINFO & 0x10) (*ModeIdIndex)++;   /* 400 lines */
          /* else 350 lines */
      }
      if(*ModeNo <= 3) {
         if(!(VGAINFO & 0x80)) (*ModeIdIndex)++;
         if(VGAINFO & 0x10)    (*ModeIdIndex)++; /* 400 lines  */
         /* else 350 lines  */
      }
      /* else 200 lines  */

   } else {

      for (*ModeIdIndex=0;;(*ModeIdIndex)++) {
         if (SiS_Pr->SiS_EModeIDTable[*ModeIdIndex].Ext_ModeID == (*ModeNo)) break;
         if (SiS_Pr->SiS_EModeIDTable[*ModeIdIndex].Ext_ModeID == 0xFF)   return FALSE;
      }

   }
   return TRUE;
}

/* For SiS 300 oem util: Search VBModeID */
BOOLEAN
SiS_SearchVBModeID(SiS_Private *SiS_Pr, UCHAR *ROMAddr, USHORT *ModeNo)
{
   USHORT ModeIdIndex;
   UCHAR VGAINFO = SiS_Pr->SiS_VGAINFO;

   if(*ModeNo <= 5) *ModeNo |= 1;

   for(ModeIdIndex=0; ; ModeIdIndex++) {
       if (SiS_Pr->SiS_VBModeIDTable[ModeIdIndex].ModeID == *ModeNo) break;
       if (SiS_Pr->SiS_VBModeIDTable[ModeIdIndex].ModeID == 0xFF)   return FALSE;
   }

   if(*ModeNo != 0x07) {
        if(*ModeNo > 0x03) return ((BOOLEAN)ModeIdIndex);
	if(VGAINFO & 0x80) return ((BOOLEAN)ModeIdIndex);
	ModeIdIndex++;
   }
   if(VGAINFO & 0x10) ModeIdIndex++;   /* 400 lines */
	                               /* else 350 lines */
   return ((BOOLEAN)ModeIdIndex);
}

/* TW: Checked against 630/301B, 315 1.09 and 650/301LVx 1.10.6s BIOS */
/* TW: Modified */
BOOLEAN
SiS_CheckMemorySize(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                    USHORT ModeNo,USHORT ModeIdIndex)
{
  USHORT memorysize,modeflag;
  ULONG  temp;

  if (ModeNo<=0x13) {
      modeflag = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_ModeFlag;
  } else {
      modeflag = SiS_Pr->SiS_EModeIDTable[ModeIdIndex].Ext_ModeFlag;
  }

  memorysize = modeflag & MemoryInfoFlag;
  memorysize >>= MemorySizeShift;			/* Get required memory size */
  memorysize++;

  temp = GetDRAMSize(SiS_Pr, HwDeviceExtension);       	/* Get adapter memory size */
  temp /= (1024*1024);   				/* (in MB) */

  if(temp < memorysize) return(FALSE);
  else return(TRUE);
}

UCHAR
SiS_GetModePtr(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex)
{
   UCHAR index;

   if(ModeNo<=0x13) {
     	index = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_StTableIndex;
   } else {
     	if(SiS_Pr->SiS_ModeType <= 0x02) index=0x1B;    /* 02 -> ModeEGA  */
     	else index=0x0F;
   }
   return index;
}

/* TW: Checked against 300, 650/LVDS (1.10.07, 1.10a) and 650/301LV BIOS */
void
SiS_SetSeqRegs(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT StandTableIndex)
{
   UCHAR SRdata;
   USHORT i;

   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x00,0x03);           	/* Set SR0  */

   SRdata = SiS_Pr->SiS_StandTable[StandTableIndex].SR[0];

   if(SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV) {
      	if(SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA) {
        	SRdata |= 0x01;
        }
   }
   if(SiS_Pr->SiS_IF_DEF_LVDS == 1) {
     if(SiS_Pr->SiS_IF_DEF_CH70xx != 0) {
       if(SiS_Pr->SiS_VBInfo & SetCRT2ToTV) {
         if(SiS_Pr->SiS_VBInfo & SetInSlaveMode) {
           SRdata |= 0x01;        			/* 8 dot clock  */
         }
       }
     }
     if(SiS_Pr->SiS_VBInfo & SetCRT2ToLCD) {
       if(SiS_Pr->SiS_VBInfo & SetInSlaveMode) {
         SRdata |= 0x01;          			/* 8 dot clock  */
       }
     }
   }

   SRdata |= 0x20;                			/* screen off  */

   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x01,SRdata);

   for(i=02;i<=04;i++) {
       	SRdata = SiS_Pr->SiS_StandTable[StandTableIndex].SR[i-1];
     	SiS_SetReg1(SiS_Pr->SiS_P3c4,i,SRdata);
   }
}

/* Checked against 300, 650/301LVx 1.10.6s and 650/LVDS 1.10.07 BIOS */
void
SiS_SetMiscRegs(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT StandTableIndex)
{
   UCHAR Miscdata;

   Miscdata = SiS_Pr->SiS_StandTable[StandTableIndex].MISC;

   if(SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV) {
      if(SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA) {
        Miscdata |= 0x0C;
      }
    }

   SiS_SetReg3(SiS_Pr->SiS_P3c2,Miscdata);
}

/* Checked against 300, 650/LVDS (1.10.07) and 650/301LVx (1.10.6s) BIOS (630 code still there!) */
void
SiS_SetCRTCRegs(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                USHORT StandTableIndex)
{
  UCHAR CRTCdata;
  USHORT i;

  SiS_SetRegAND(SiS_Pr->SiS_P3d4,0x11,0x7f);                       /* Unlock CRTC */

  for(i=0;i<=0x18;i++) {
     CRTCdata=SiS_Pr->SiS_StandTable[StandTableIndex].CRTC[i];
     SiS_SetReg1(SiS_Pr->SiS_P3d4,i,CRTCdata);                     /* Set CRTC(3d4) */
  }
  if( ( (HwDeviceExtension->jChipType == SIS_630) ||
        (HwDeviceExtension->jChipType == SIS_730) )  &&
      (HwDeviceExtension->jChipRevision >= 0x30) ) {       	   /* for 630S0 */
    if(SiS_Pr->SiS_VBInfo & SetInSlaveMode) {
      if(SiS_Pr->SiS_VBInfo & (SetCRT2ToLCD | SetCRT2ToTV)) {
        SiS_SetReg1(SiS_Pr->SiS_P3d4,0x18,0xFE);
      }
    }
  }
}

/* TW: Checked against 300, 650/LVDS (1.10.07), 650/301LVx (1.10.6s) and 630/301B BIOS */
void
SiS_SetATTRegs(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT StandTableIndex,USHORT ModeNo,
               PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   UCHAR ARdata;
   USHORT i;

   for(i=0;i<=0x13;i++) {
    ARdata = SiS_Pr->SiS_StandTable[StandTableIndex].ATTR[i];
    if(i == 0x13) {
      if(SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV) {
        if(SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA)  ARdata=0;
      }
      if(SiS_Pr->SiS_IF_DEF_LVDS == 1) {
        if(SiS_Pr->SiS_IF_DEF_CH70xx != 0) {
          if(SiS_Pr->SiS_VBInfo & SetCRT2ToTV) {
            if(SiS_Pr->SiS_VBInfo & SetInSlaveMode) ARdata=0;
          }
        }
      }
      if(SiS_Pr->SiS_VBInfo & SetCRT2ToLCD) {
        if(HwDeviceExtension->jChipType >= SIS_315H) {
	  /* TW: From 650/LVDS 1.10.07, 1.10a; 650/301LVx 1.10.6s */
	  ARdata = 0;
	} else {
          if(SiS_Pr->SiS_VBInfo & SetInSlaveMode) {
	     ARdata=0;
          }
	}
      }
    }
    SiS_GetReg2(SiS_Pr->SiS_P3da);                              /* reset 3da  */
    SiS_SetReg3(SiS_Pr->SiS_P3c0,i);                            /* set index  */
    SiS_SetReg3(SiS_Pr->SiS_P3c0,ARdata);                       /* set data   */
   }
   SiS_GetReg2(SiS_Pr->SiS_P3da);                               /* reset 3da  */
   SiS_SetReg3(SiS_Pr->SiS_P3c0,0x14);                          /* set index  */
   SiS_SetReg3(SiS_Pr->SiS_P3c0,0x00);                          /* set data   */

   SiS_GetReg2(SiS_Pr->SiS_P3da);                               /* Enable Attribute  */
   SiS_SetReg3(SiS_Pr->SiS_P3c0,0x20);
}

/* TW: Checked against 300, 650/LVDS (1.10.07, 1.10a) and 650/301LV BIOS */
void
SiS_SetGRCRegs(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT StandTableIndex)
{
   UCHAR GRdata;
   USHORT i;

   for(i=0;i<=0x08;i++) {
     GRdata = SiS_Pr->SiS_StandTable[StandTableIndex].GRC[i]; 	  	/* Get GR from file */
     SiS_SetReg1(SiS_Pr->SiS_P3ce,i,GRdata);                    /* Set GR(3ce) */
   }

   if(SiS_Pr->SiS_ModeType > ModeVGA) {
     SiS_SetRegAND(SiS_Pr->SiS_P3ce,0x05,0xBF);			/* 256 color disable */
   }
}

/* TW: Checked against 650/LVDS (1.10.07, 1.10a), 650/301LVx (1.10.6s) and 630/301B BIOS */
void
SiS_ClearExt1Regs(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  USHORT i;

  for(i=0x0A;i<=0x0E;i++) SiS_SetReg1(SiS_Pr->SiS_P3c4,i,0x00);      /* Clear SR0A-SR0E */

  /* TW: New from 650/LVDS/301LV BIOSes: */
  if(HwDeviceExtension->jChipType >= SIS_315H) {
     SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x37,0xFE);
  }
}

/* TW: Checked against 300, 650/LVDS (1.10.07) and 650/301LV BIOS */
void
SiS_SetSync(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT RefreshRateTableIndex)
{
  USHORT sync;
  USHORT temp;

  sync = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_InfoFlag >> 8;

  sync &= 0xC0;
  temp = 0x2F | sync;
  SiS_SetReg3(SiS_Pr->SiS_P3c2,temp);                                 /* Set Misc(3c2) */
}

/* TW: Checked against 300, 650/LVDS (1.10.07) and 650/301LVx (1.10.6s) BIOS */
void
SiS_SetCRT1CRTC(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex,
                USHORT RefreshRateTableIndex,
		PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  UCHAR  index;
  USHORT tempah,i,modeflag,j;
#ifdef SIS315H
  USHORT temp;
  USHORT ResInfo,DisplayType;
  const SiS_LCDACRT1DataStruct *LCDACRT1Ptr = NULL;
#endif

  SiS_SetRegAND(SiS_Pr->SiS_P3d4,0x11,0x7f);		/*unlock cr0-7  */

  if(ModeNo<=0x13) {
        modeflag = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_ModeFlag;
  } else {
        modeflag = SiS_Pr->SiS_EModeIDTable[ModeIdIndex].Ext_ModeFlag;
  }

  if((SiS_Pr->SiS_IF_DEF_LVDS == 0) && (SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA)) {

#ifdef SIS315H

     /* LCDA */

     temp = SiS_GetLCDACRT1Ptr(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,
                       RefreshRateTableIndex,&ResInfo,&DisplayType);

     switch(DisplayType) {
      case Panel_800x600       : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT1800x600_1;           break;
      case Panel_1024x768      : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11024x768_1;          break;
      case Panel_1280x1024     : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11280x1024_1;         break;
      case Panel_1400x1050     : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11400x1050_1;         break;
      case Panel_1600x1200     : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11600x1200_1;         break;
      case Panel_800x600 + 16  : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT1800x600_1_H;         break;
      case Panel_1024x768 + 16 : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11024x768_1_H;        break;
      case Panel_1280x1024 + 16: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11280x1024_1_H;       break;
      case Panel_1400x1050 + 16: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11400x1050_1_H;       break;
      case Panel_1600x1200 + 16: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11600x1200_1_H;       break;
      case Panel_800x600 + 32  : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT1800x600_2;           break;
      case Panel_1024x768 + 32 : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11024x768_2;          break;
      case Panel_1280x1024 + 32: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11280x1024_2;         break;
      case Panel_1400x1050 + 32: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11400x1050_2;         break;
      case Panel_1600x1200 + 32: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11600x1200_2;         break;
      case Panel_800x600 + 48  : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT1800x600_2_H;         break;
      case Panel_1024x768 + 48 : LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11024x768_2_H;        break;
      case Panel_1280x1024 + 48: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11280x1024_2_H;       break;
      case Panel_1400x1050 + 48: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11400x1050_2_H;       break;
      case Panel_1600x1200 + 48: LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11600x1200_2_H;       break;
      default:                   LCDACRT1Ptr = SiS_Pr->SiS_LCDACRT11024x768_1;          break;
     }

     tempah = (LCDACRT1Ptr+ResInfo)->CR[0];
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x00,tempah);
     for(i=0x01,j=1;i<=0x07;i++,j++){
       tempah = (LCDACRT1Ptr+ResInfo)->CR[j];
       SiS_SetReg1(SiS_Pr->SiS_P3d4,i,tempah);
     }
     for(i=0x10,j=8;i<=0x12;i++,j++){
       tempah = (LCDACRT1Ptr+ResInfo)->CR[j];
       SiS_SetReg1(SiS_Pr->SiS_P3d4,i,tempah);
     }
     for(i=0x15,j=11;i<=0x16;i++,j++){
       tempah =(LCDACRT1Ptr+ResInfo)->CR[j];
       SiS_SetReg1(SiS_Pr->SiS_P3d4,i,tempah);
     }
     for(i=0x0A,j=13;i<=0x0C;i++,j++){
       tempah = (LCDACRT1Ptr+ResInfo)->CR[j];
       SiS_SetReg1(SiS_Pr->SiS_P3c4,i,tempah);
     }

     tempah = (LCDACRT1Ptr+ResInfo)->CR[16];
     tempah &= 0x0E0;
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x0E,tempah);

     tempah = (LCDACRT1Ptr+ResInfo)->CR[16];
     tempah &= 0x01;
     tempah <<= 5;
     if(modeflag & DoubleScanMode)  tempah |= 0x080;
     SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x09,~0x020,tempah);

#endif

  } else {

     /* LVDS, 301, 301B, 301LV, 302LV, ... (non-LCDA) */

     index = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_CRT1CRTC;  	/* Get index */
     if(HwDeviceExtension->jChipType < SIS_315H) {
        index &= 0x3F;
     }

     for(i=0,j=0;i<=07;i++,j++) {
       tempah=SiS_Pr->SiS_CRT1Table[index].CR[i];
       SiS_SetReg1(SiS_Pr->SiS_P3d4,j,tempah);
     }
     for(j=0x10;i<=10;i++,j++) {
       tempah=SiS_Pr->SiS_CRT1Table[index].CR[i];
       SiS_SetReg1(SiS_Pr->SiS_P3d4,j,tempah);
     }
     for(j=0x15;i<=12;i++,j++) {
       tempah=SiS_Pr->SiS_CRT1Table[index].CR[i];
       SiS_SetReg1(SiS_Pr->SiS_P3d4,j,tempah);
     }
     for(j=0x0A;i<=15;i++,j++) {
       tempah=SiS_Pr->SiS_CRT1Table[index].CR[i];
       SiS_SetReg1(SiS_Pr->SiS_P3c4,j,tempah);
     }

     tempah = SiS_Pr->SiS_CRT1Table[index].CR[16];
     tempah &= 0xE0;
     SiS_SetReg1(SiS_Pr->SiS_P3c4,0x0E,tempah);

     tempah = SiS_Pr->SiS_CRT1Table[index].CR[16];
     tempah &= 0x01;
     tempah <<= 5;
     if(modeflag & DoubleScanMode)  tempah |= 0x80;
     SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x09,0xDF,tempah);

  }

  if(SiS_Pr->SiS_ModeType > ModeVGA) SiS_SetReg1(SiS_Pr->SiS_P3d4,0x14,0x4F);
}

BOOLEAN
SiS_GetLCDACRT1Ptr(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex,
		   USHORT RefreshRateTableIndex,USHORT *ResInfo,
		   USHORT *DisplayType)
 {
  USHORT tempbx=0,modeflag=0;
  USHORT CRT2CRTC=0;

  if(ModeNo<=0x13) {
  	modeflag = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_ModeFlag;
  	CRT2CRTC = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_CRT2CRTC;
  } else {
  	modeflag = SiS_Pr->SiS_EModeIDTable[ModeIdIndex].Ext_ModeFlag;
  	CRT2CRTC = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_CRT2CRTC;
  }

  tempbx = SiS_Pr->SiS_LCDResInfo;

  if(SiS_Pr->SiS_LCDInfo & LCDNonExpanding) tempbx += 32;
  if(modeflag & HalfDCLK)                   tempbx += 16;

  *ResInfo = CRT2CRTC & 0x3F;
  *DisplayType = tempbx;

  return 1;
}

/* TW: Set offset and pitch - partly overruled by SetPitch() in XF86 */
/* TW: Checked against 650/LVDS (1.10.07), 650/301LV and 315 BIOS */
void
SiS_SetCRT1Offset(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex,
                  USHORT RefreshRateTableIndex,
		  PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   USHORT temp, DisplayUnit, infoflag;

   infoflag = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_InfoFlag;

   DisplayUnit = SiS_GetOffset(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,
                     RefreshRateTableIndex,HwDeviceExtension);

   temp = (DisplayUnit >> 8) & 0x0f;
   SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x0E,0xF0,temp);

   temp = DisplayUnit & 0xFF;
   SiS_SetReg1(SiS_Pr->SiS_P3d4,0x13,temp);

   if(infoflag & InterlaceMode) DisplayUnit >>= 1;

   DisplayUnit <<= 5;
   temp = (DisplayUnit & 0xff00) >> 8;
   if (DisplayUnit & 0xff) temp++;
   temp++;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x10,temp);
}

/* TW: New from 650/LVDS 1.10.07, 630/301B and 630/LVDS BIOS */
void
SiS_ResetCRT1VCLK(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
   USHORT index;

   /* TW: We only need to do this if Panel Link is to be
    *     initialized, thus on 630/LVDS/301B, and 650/LVDS
    */
   if(HwDeviceExtension->jChipType >= SIS_315H) {
       if (SiS_Pr->SiS_IF_DEF_LVDS == 0)  return;
   } else {
       if( (SiS_Pr->SiS_IF_DEF_LVDS == 0) &&
           (!(SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV)) ) {
	   return;
      }
   }

   if(HwDeviceExtension->jChipType >= SIS_315H) {
   	SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x31,0xCF,0x20);
   } else {
   	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x31,0x20);
   }
   index = 1;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2B,SiS_Pr->SiS_VCLKData[index].SR2B);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2C,SiS_Pr->SiS_VCLKData[index].SR2C);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2D,0x80);
   if(HwDeviceExtension->jChipType >= SIS_315H) {
   	SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x31,0xcf,0x10);
   } else {
   	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x31,0x10);
   }
   index = 0;
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2B,SiS_Pr->SiS_VCLKData[index].SR2B);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2C,SiS_Pr->SiS_VCLKData[index].SR2C);
   SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2D,0x80);
}

/* TW: Checked against 300, 650/LVDS, 650/301LVx, 315, 630/301B, 630/LVDS BIOS */
void
SiS_SetCRT1VCLK(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex,
                PSIS_HW_DEVICE_INFO HwDeviceExtension,
		USHORT RefreshRateTableIndex)
{
  USHORT  index;

  index = SiS_GetVCLK2Ptr(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,
	                  RefreshRateTableIndex,HwDeviceExtension);

  if( (SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV)
                       && (SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA) ){

    	SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x31,0xCF);

    	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2B,SiS_Pr->SiS_VBVCLKData[index].Part4_A);
    	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2C,SiS_Pr->SiS_VBVCLKData[index].Part4_B);

    	if(HwDeviceExtension->jChipType >= SIS_315H) {
		SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2D,0x01);
   	} else {
    		SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2D,0x80);
    	}

  } else {

	if(HwDeviceExtension->jChipType >= SIS_315H) {
	    SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x31,0xCF);
	} else {
	    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x31,0x00);
	}

    	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2B,SiS_Pr->SiS_VCLKData[index].SR2B);
    	SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2C,SiS_Pr->SiS_VCLKData[index].SR2C);

    	if(HwDeviceExtension->jChipType >= SIS_315H) {
	    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2D,0x01);
	} else {
      	    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x2D,0x80);
        }
  }
}


/* TW: Checked against 300, 630/LVDS, 650/LVDS and 315 BIOS */
void
SiS_SetCRT1ModeRegs(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                    USHORT ModeNo,USHORT ModeIdIndex,USHORT RefreshRateTableIndex)
{
  USHORT data,data2,data3;
  USHORT infoflag=0,modeflag;
  USHORT resindex,xres;

  if(ModeNo > 0x13) {
    	modeflag = SiS_Pr->SiS_EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    	infoflag = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_InfoFlag;
  } else {
    	modeflag = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_ModeFlag;
  }

  SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x1F,0x3F); 		/* DAC pedestal */

  if(ModeNo > 0x13) data = infoflag;
  else data = 0;

  data2 = 0;
  if(ModeNo > 0x13) {
    if(SiS_Pr->SiS_ModeType > 0x02) {
       data2 |= 0x02;
       data3 = (SiS_Pr->SiS_ModeType - ModeVGA) << 2;
       data2 |= data3;
    }
  }
  if(data & InterlaceMode) data2 |= 0x20;
  SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x06,0xC0, data2);

  resindex = SiS_GetResInfo(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex);
  if(ModeNo <= 0x13) {
      	xres = SiS_Pr->SiS_StResInfo[resindex].HTotal;
  } else {
      	xres = SiS_Pr->SiS_ModeResInfo[resindex].HTotal;
  }

  if(HwDeviceExtension->jChipType != SIS_300) {
     data = 0x0000;
     if(infoflag & InterlaceMode) {
        if(xres == 1024) data = 0x0035;
        else data = 0x0048;
     }
     data2 = data & 0x00FF;
     SiS_SetReg1(SiS_Pr->SiS_P3d4,0x19,data2);
     data2 = (data & 0xFF00) >> 8;
     SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x1a,0xFC,data2);
  }

  if(modeflag & HalfDCLK) {
     SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x01,0x08);
  }

  if(HwDeviceExtension->jChipType == SIS_300) {
     if(modeflag & LineCompareOff) {
        SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x0F,0x08);
     } else {
        SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x0F,0xF7);
     }
  } else if(HwDeviceExtension->jChipType < SIS_315H) {
     if(modeflag & LineCompareOff) {
        SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x0F,0xB7,0x08);
     } else {
        SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x0F,0xB7);
     }
     /* 630 BIOS does something for mode 0x12 here */
  } else {
     if(modeflag & LineCompareOff) {
        SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x0F,0xB7,0x08);
     } else {
        SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x0F,0xB7);
     }
  }

  if(HwDeviceExtension->jChipType != SIS_300) {
     if(SiS_Pr->SiS_ModeType == ModeEGA) {
        if(ModeNo > 0x13) {
  	   SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x0F,0x40);
        }
     }
  }

#ifdef SIS315H
  /* TW: 315 BIOS sets SR17 at this point */
  if(HwDeviceExtension->jChipType == SIS_315PRO) {
      data = SiS_Get310DRAMType(SiS_Pr,ROMAddr,HwDeviceExtension);
      data = SiS_Pr->SiS_SR15[2][data];
      if(SiS_Pr->SiS_ModeType == ModeText) {
          data &= 0xc7;
      } else {
          data2 = SiS_GetOffset(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,
                                RefreshRateTableIndex,HwDeviceExtension);
	  data2 >>= 1;
	  if(infoflag & InterlaceMode) data2 >>= 1;
	  data3 = SiS_GetColorDepth(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex);
	  data3 >>= 1;
	  if(data3 == 0) data3++;
	  data2 /= data3;
	  if(data2 >= 0x50) {
	      data &= 0x0f;
	      data |= 0x50;
	  }
      }
      SiS_SetReg1(SiS_Pr->SiS_P3c4,0x17,data);
  }
#endif

  data = 0x60;
  if(SiS_Pr->SiS_ModeType != ModeText) {
      data ^= 0x60;
      if(SiS_Pr->SiS_ModeType != ModeEGA) {
        data ^= 0xA0;
      }
  }
  SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x21,0x1F,data);

  SiS_SetVCLKState(SiS_Pr,ROMAddr,HwDeviceExtension,ModeNo,RefreshRateTableIndex,ModeIdIndex);

#ifdef SIS315H
  if(HwDeviceExtension->jChipType >= SIS_315H) {
    if(SiS_GetReg1(SiS_Pr->SiS_P3d4,0x31) & 0x40) {
        SiS_SetReg1(SiS_Pr->SiS_P3d4,0x52,0x2c);
    } else {
        SiS_SetReg1(SiS_Pr->SiS_P3d4,0x52,0x6c);
    }
  }
#endif
}

/* TW: Checked against 300, 315, 650/LVDS, 650/301LVx, 630/301B and 630/LVDS BIOS */
void
SiS_SetVCLKState(SiS_Private *SiS_Pr, UCHAR *ROMAddr,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                 USHORT ModeNo,USHORT RefreshRateTableIndex,
                 USHORT ModeIdIndex)
{
  USHORT data, data2=0;
  USHORT VCLK, index=0;

  if (ModeNo <= 0x13) VCLK = 0;
  else {
     index = SiS_GetVCLK2Ptr(SiS_Pr,ROMAddr,ModeNo,ModeIdIndex,
	               RefreshRateTableIndex,HwDeviceExtension);
     VCLK = SiS_Pr->SiS_VCLKData[index].CLOCK;
  }

  if(HwDeviceExtension->jChipType < SIS_315H) {		/* 300 series */

    data2 = 0x00;
    if(VCLK > 150) data2 |= 0x80;
    SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x07,0x7B,data2); 	/* DAC speed */

    data2 = 0x00;
    if(VCLK >= 150) data2 |= 0x08;       	/* VCLK > 150 */
    SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x32,0xF7,data2);

  } else { 						/* 310/325 series */

    data = 0;
    if(VCLK >= 166) data |= 0x0c;         	/* TW: Was 200; is 166 in 650 and 315 BIOSes */
    SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x32,0xf3,data);

    if(VCLK >= 166) {				/* TW: Was 200, is 166 in 650 and 315 BIOSes */
       SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x1f,0xe7);
    }
  }

  data2 = 0x03;
  if((VCLK >= 135) && (VCLK < 160)) data2 = 0x02;
  if((VCLK >= 160) && (VCLK < 260)) data2 = 0x01;
  if(VCLK >= 260) data2 = 0x00;
  /* disable 24bit palette RAM gamma correction  */
  if(HwDeviceExtension->jChipType == SIS_540) {
    	if((VCLK == 203) || (VCLK < 234)) data2 = 0x02;
  }
  if(HwDeviceExtension->jChipType < SIS_315H) {
      SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x07,0xFC,data2);  	/* DAC speed */
  } else {
      if(HwDeviceExtension->jChipType > SIS_315PRO) {
         /* TW: This "if" is done in 650/LVDS/301LV BIOSes; Not in 315 BIOS */
         if(ModeNo > 0x13) data2 &= 0xfc;
      }
      SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x07,0xF8,data2);  	/* DAC speed */
  }
}

/* TW: Checked against 650/301LVx 1.10.6s, 315, 630/301B BIOS */
void
SiS_LoadDAC(SiS_Private *SiS_Pr,PSIS_HW_DEVICE_INFO HwDeviceExtension,
            UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex)
{
   USHORT data,data2;
   USHORT time,i,j,k;
   USHORT m,n,o;
   USHORT si,di,bx,dl;
   USHORT al,ah,dh;
   USHORT DACAddr, DACData, shiftflag;
   const USHORT *table = NULL;

   if (ModeNo<=0x13)
        data = SiS_Pr->SiS_SModeIDTable[ModeIdIndex].St_ModeFlag;
   else
        data = SiS_Pr->SiS_EModeIDTable[ModeIdIndex].Ext_ModeFlag;


	data &= DACInfoFlag;
	time = 64;
	if(data == 0x00) table = SiS_MDA_DAC;
	if(data == 0x08) table = SiS_CGA_DAC;
	if(data == 0x10) table = SiS_EGA_DAC;
	if(data == 0x18) {
	   time = 256;
	   table = SiS_VGA_DAC;
	}
	if(time == 256) j = 16;
	else            j = time;

	if( ( (HwDeviceExtension->jChipType < SIS_315H) &&         /* 630/301B */
	      (SiS_Pr->SiS_VBType & VB_SIS301BLV302BLV) &&
	      (SiS_Pr->SiS_VBInfo & SetCRT2ToLCD) )         ||
	    (SiS_Pr->SiS_VBInfo & SetCRT2ToLCDA)            ||     /* LCDA */
	    (!(SiS_Pr->SiS_SetFlag & ProgrammingCRT2)) ) {         /* Programming CRT1 */
	   DACAddr = SiS_Pr->SiS_P3c8;
	   DACData = SiS_Pr->SiS_P3c9;
	   shiftflag = 0;
	   SiS_SetReg3(SiS_Pr->SiS_P3c6,0xFF);
	} else {
	   shiftflag = 1;
	   DACAddr = SiS_Pr->SiS_Part5Port;
	   DACData = SiS_Pr->SiS_Part5Port + 1;
	}

	SiS_SetReg3(DACAddr,0x00);

	for(i=0; i<j; i++) {
	   data = table[i];
	   for(k=0; k<3; k++) {
		data2 = 0;
		if(data & 0x01) data2 = 0x2A;
		if(data & 0x02) data2 += 0x15;
		if(shiftflag) data2 <<= 2;
		SiS_SetReg3(DACData,data2);
		data >>= 2;
	   }
	}

	if(time == 256) {
	   for(i = 16; i < 32; i++) {
		data = table[i];
		if(shiftflag) data <<= 2;
		for(k=0; k<3; k++) SiS_SetReg3(DACData,data);
	   }
	   si = 32;
	   for(m = 0; m < 9; m++) {
	      di = si;
	      bx = si + 4;
	      dl = 0;
	      for(n = 0; n < 3; n++) {
		 for(o = 0; o < 5; o++) {
		    dh = table[si];
		    ah = table[di];
		    al = table[bx];
		    si++;
		    SiS_WriteDAC(SiS_Pr,DACData,shiftflag,dl,ah,al,dh);
		 }
		 si -= 2;
		 for(o = 0; o < 3; o++) {
		    dh = table[bx];
		    ah = table[di];
		    al = table[si];
		    si--;
		    SiS_WriteDAC(SiS_Pr,DACData,shiftflag,dl,ah,al,dh);
		 }
		 dl++;
	      }            /* for n < 3 */
	      si += 5;
	   }               /* for m < 9 */
	}

}

void
SiS_WriteDAC(SiS_Private *SiS_Pr, USHORT DACData, USHORT shiftflag,
             USHORT dl, USHORT ah, USHORT al, USHORT dh)
{
  USHORT temp;
  USHORT bh,bl;

  bh = ah;
  bl = al;
  if(dl != 0) {
    temp = bh;
    bh = dh;
    dh = temp;
    if(dl == 1) {
       temp = bl;
       bl = dh;
       dh = temp;
    } else {
       temp = bl;
       bl = bh;
       bh = temp;
    }
  }
  if(shiftflag) {
     dh <<= 2;
     bh <<= 2;
     bl <<= 2;
  }
  SiS_SetReg3(DACData,(USHORT)dh);
  SiS_SetReg3(DACData,(USHORT)bh);
  SiS_SetReg3(DACData,(USHORT)bl);
}

ULONG
GetDRAMSize(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  ULONG   AdapterMemorySize = 0;
#ifdef SIS315H
  USHORT  counter;
#endif

#ifdef SIS315H
  if ((HwDeviceExtension->jChipType == SIS_315H) ||
      (HwDeviceExtension->jChipType == SIS_315PRO)) {
    	counter = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
	AdapterMemorySize = 1 << ((counter & 0xF0) >> 4);
	counter >>= 2;
	counter &= 0x03;
	if(counter == 0x02) {
		AdapterMemorySize += (AdapterMemorySize / 2);      /* DDR asymetric */
	} else if(counter != 0) {
		AdapterMemorySize <<= 1;                           /* SINGLE_CHANNEL_2_RANK or DUAL_CHANNEL_1_RANK */
	}
	AdapterMemorySize *= (1024*1024);

  } else if((HwDeviceExtension->jChipType == SIS_550) ||
            (HwDeviceExtension->jChipType == SIS_640) ||
            (HwDeviceExtension->jChipType == SIS_740) ||
            (HwDeviceExtension->jChipType == SIS_650)) {
      		counter = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14) & 0x3F;
      		counter++;
      		AdapterMemorySize = counter * 4;
      		AdapterMemorySize *= (1024*1024);
  }
#endif

#ifdef SIS300
  if ((HwDeviceExtension->jChipType==SIS_300) ||
      (HwDeviceExtension->jChipType==SIS_540) ||
      (HwDeviceExtension->jChipType==SIS_630) ||
      (HwDeviceExtension->jChipType==SIS_730)) {
      	AdapterMemorySize = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14) & 0x3F;
      	AdapterMemorySize++;
      	AdapterMemorySize *= (1024*1024);
  }
#endif

  return AdapterMemorySize;
}

#ifndef LINUX_XF86
void
SiS_ClearBuffer(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,USHORT ModeNo)
{
  PVOID   VideoMemoryAddress = (PVOID)HwDeviceExtension->pjVideoMemoryAddress;
  ULONG   AdapterMemorySize  = (ULONG)HwDeviceExtension->ulVideoMemorySize;
  PUSHORT pBuffer;
  int i;

  if (SiS_Pr->SiS_ModeType>=ModeEGA) {
    if(ModeNo > 0x13) {
      AdapterMemorySize = GetDRAMSize(SiS_Pr, HwDeviceExtension);
      SiS_SetMemory(VideoMemoryAddress,AdapterMemorySize,0);
    } else {
      pBuffer = VideoMemoryAddress;
      for(i=0; i<0x4000; i++)
         pBuffer[i] = 0x0000;
    }
  } else {
    pBuffer = VideoMemoryAddress;
    if (SiS_Pr->SiS_ModeType < ModeCGA) {
      for(i=0; i<0x4000; i++)
         pBuffer[i] = 0x0720;
    } else {
      SiS_SetMemory(VideoMemoryAddress,0x8000,0);
    }
  }
}
#endif

void
SiS_DisplayOn(SiS_Private *SiS_Pr)
{
   SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x01,0xDF,0x00);
}

void
SiS_DisplayOff(SiS_Private *SiS_Pr)
{
   SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x01,0xDF,0x20);
}


/* ========================================== */
/*  SR CRTC GR */
void
SiS_SetReg1(USHORT port, USHORT index, USHORT data)
{
   OutPortByte(port,index);
   OutPortByte(port+1,data);
}

/* ========================================== */
/*  AR(3C0) */
void
SiS_SetReg2(SiS_Private *SiS_Pr, USHORT port, USHORT index, USHORT data)
{
   InPortByte(port+0x3da-0x3c0);
   OutPortByte(SiS_Pr->SiS_P3c0,index);
   OutPortByte(SiS_Pr->SiS_P3c0,data);
   OutPortByte(SiS_Pr->SiS_P3c0,0x20);
}

void
SiS_SetReg3(USHORT port, USHORT data)
{
   OutPortByte(port,data);
}

void
SiS_SetReg4(USHORT port, ULONG data)
{
   OutPortLong(port,data);
}

UCHAR SiS_GetReg1(USHORT port, USHORT index)
{
   UCHAR   data;

   OutPortByte(port,index);
   data = InPortByte(port+1);

   return(data);
}

UCHAR
SiS_GetReg2(USHORT port)
{
   UCHAR   data;

   data= InPortByte(port);

   return(data);
}

ULONG
SiS_GetReg3(USHORT port)
{
   ULONG   data;

   data = InPortLong(port);

   return(data);
}

void
SiS_ClearDAC(SiS_Private *SiS_Pr, ULONG port)
{
   int i;

   OutPortByte(port, 0);
   port++;
   for (i=0; i < (256 * 3); i++) {
      OutPortByte(port, 0);
   }

}


/* TW: Checked against 650/LVDS (1.10.07), 650/301LVx (1.10.6s) and 315 BIOS */
#ifdef SIS315H
void
SiS_SetCRT1FIFO_310(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,USHORT ModeIdIndex,
                PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  USHORT modeflag;

  SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x3D,0xFE);  /* disable auto-threshold */

  if(ModeNo > 0x13) {
    modeflag = SiS_Pr->SiS_EModeIDTable[ModeIdIndex].Ext_ModeFlag;
    if( (!(modeflag & DoubleScanMode)) || (!(modeflag & HalfDCLK))) {
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x08,0x34);
       SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x09,0xF0);
       SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x3D,0x01);
    } else {
       SiS_SetReg1(SiS_Pr->SiS_P3c4,0x08,0xAE);
       SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x09,0xF0);
    }
  } else {
    SiS_SetReg1(SiS_Pr->SiS_P3c4,0x08,0xAE);
    SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x09,0xF0);
  }
}
#endif

#ifdef SIS300
void
SiS_SetCRT1FIFO_300(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                    USHORT RefreshRateTableIndex)
{
  USHORT  ThresholdLow = 0;
  USHORT  index, VCLK, MCLK, colorth=0;
  USHORT  tempah, temp;

  if(ModeNo > 0x13) {

     index = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_CRTVCLK;
     index &= 0x3F;
     VCLK = SiS_Pr->SiS_VCLKData[index].CLOCK;             /* Get VCLK  */

     switch (SiS_Pr->SiS_ModeType - ModeEGA) {     /* Get half colordepth */
        case 0 : colorth = 1; break;
        case 1 : colorth = 1; break;
        case 2 : colorth = 2; break;
        case 3 : colorth = 2; break;
        case 4 : colorth = 3; break;
        case 5 : colorth = 4; break;
     }

     index = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x3A);
     index &= 0x07;
     MCLK = SiS_Pr->SiS_MCLKData_0[index].CLOCK;           /* Get MCLK  */

     tempah = SiS_GetReg1(SiS_Pr->SiS_P3d4,0x35);
     tempah &= 0xc3;
     SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x16,0x3c,tempah);

     do {
        ThresholdLow = SiS_CalcDelay(SiS_Pr, ROMAddr, VCLK, colorth, MCLK);
        ThresholdLow++;
        if(ThresholdLow < 0x13) break;
        SiS_SetRegAND(SiS_Pr->SiS_P3c4,0x16,0xfc);
        ThresholdLow = 0x13;
        tempah = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x16);
        tempah >>= 6;
        if(!(tempah)) break;
        tempah--;
        tempah <<= 6;
        SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x16,0x3f,tempah);
     } while(0);

  } else ThresholdLow = 2;

  /* Write CRT/CPU threshold low, CRT/Engine threshold high */
  temp = (ThresholdLow << 4) | 0x0f;
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x08,temp);

  temp = (ThresholdLow & 0x10) << 1;
  if(ModeNo > 0x13) temp |= 0x40;
  SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x0f,0x9f,temp);

  /* What is this? */
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x3B,0x09);

  /* Write CRT/CPU threshold high */
  temp = ThresholdLow + 3;
  if(temp > 0x0f) temp = 0x0f;
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x09,temp);
}

USHORT
SiS_CalcDelay(SiS_Private *SiS_Pr, UCHAR *ROMAddr, USHORT VCLK, USHORT colordepth, USHORT MCLK)
{
  USHORT tempax, tempbx;

  tempbx = SiS_DoCalcDelay(SiS_Pr, MCLK, VCLK, colordepth, 0);
  tempax = SiS_DoCalcDelay(SiS_Pr, MCLK, VCLK, colordepth, 1);
  if(tempax < 4) tempax = 4;
  tempax -= 4;
  if(tempbx < tempax) tempbx = tempax;
  return(tempbx);
}

USHORT
SiS_DoCalcDelay(SiS_Private *SiS_Pr, USHORT MCLK, USHORT VCLK, USHORT colordepth, USHORT key)
{
  const UCHAR ThLowA[]   = { 61, 3,52, 5,68, 7,100,11,
                             43, 3,42, 5,54, 7, 78,11,
                             34, 3,37, 5,47, 7, 67,11 };

  const UCHAR ThLowB[]   = { 81, 4,72, 6,88, 8,120,12,
                             55, 4,54, 6,66, 8, 90,12,
                             42, 4,45, 6,55, 8, 75,12 };

  const UCHAR ThTiming[] = {  1, 2, 2, 3, 0, 1,  1, 2 };

  USHORT tempah, tempal, tempcl, tempbx, temp;
  ULONG  longtemp;

  tempah = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x18);
  tempah &= 0x62;
  tempah >>= 1;
  tempal = tempah;
  tempah >>= 3;
  tempal |= tempah;
  tempal &= 0x07;
  tempcl = ThTiming[tempal];
  tempbx = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x16);
  tempbx >>= 6;
  tempah = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
  tempah >>= 4;
  tempah &= 0x0c;
  tempbx |= tempah;
  tempbx <<= 1;
  if(key == 0) {
     tempal = ThLowA[tempbx + 1];
     tempal *= tempcl;
     tempal += ThLowA[tempbx];
  } else {
     tempal = ThLowB[tempbx + 1];
     tempal *= tempcl;
     tempal += ThLowB[tempbx];
  }
  longtemp = tempal * VCLK * colordepth;
  temp = longtemp % (MCLK * 16);
  longtemp /= (MCLK * 16);
  if(temp) longtemp++;
  return((USHORT)longtemp);
}


void
SiS_SetCRT1FIFO_630(SiS_Private *SiS_Pr, UCHAR *ROMAddr,USHORT ModeNo,PSIS_HW_DEVICE_INFO HwDeviceExtension,
                    USHORT RefreshRateTableIndex)
{
  USHORT  i,index,data,VCLK,MCLK,colorth=0;
  ULONG   B,eax,bl,data2;
  USHORT  ThresholdLow=0;
  UCHAR   FQBQData[]= { 0x01,0x21,0x41,0x61,0x81,
                        0x31,0x51,0x71,0x91,0xb1,
                        0x00,0x20,0x40,0x60,0x80,
                        0x30,0x50,0x70,0x90,0xb0,0xFF};

  i=0;
  if(ModeNo >= 0x13) {
    index = SiS_Pr->SiS_RefIndex[RefreshRateTableIndex].Ext_CRTVCLK;
    index &= 0x3F;
    VCLK = SiS_Pr->SiS_VCLKData[index].CLOCK;             /* Get VCLK  */

    index = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1A);
    index &= 0x07;
    MCLK = SiS_Pr->SiS_MCLKData_0[index].CLOCK;           /* Get MCLK  */

    data2 = SiS_Pr->SiS_ModeType - ModeEGA;	  /* Get half colordepth */
    switch (data2) {
        case 0 : colorth = 1; break;
        case 1 : colorth = 1; break;
        case 2 : colorth = 2; break;
        case 3 : colorth = 2; break;
        case 4 : colorth = 3; break;
        case 5 : colorth = 4; break;
    }

    do{
       B = SiS_CalcDelay2(SiS_Pr, ROMAddr, FQBQData[i]) * VCLK * colorth;
       bl = B / (MCLK * 16);

       if (B==bl*16*MCLK) {
         bl = bl + 1;
       } else {
         bl = bl + 2;
       }

       if(bl > 0x13) {
          if(FQBQData[i+1] == 0xFF) {
             ThresholdLow = 0x13;
             break;
          }
          i++;
       } else {
          ThresholdLow = bl;
          break;
       }
    } while(FQBQData[i] != 0xFF);
  }
  else {
    ThresholdLow = 0x02;
  }

  /* Write foreground and background queue */
  data2 = FQBQData[i];
  data2 = (data2 & 0xf0)>>4;
  data2 <<= 24;

#ifndef LINUX_XF86
  SiS_SetReg4(0xcf8,0x80000050);
  eax = SiS_GetReg3(0xcfc);
  eax &= 0xf0ffffff;
  eax |= data2;
  SiS_SetReg4(0xcfc,eax);
#else
  /* We use pci functions X offers. We use pcitag 0, because
   * we want to read/write to the host bridge (which is always
   * 00:00.0 on 630, 730 and 540), not the VGA device.
   */
  eax = pciReadLong(0x00000000, 0x50);
  eax &= 0xf0ffffff;
  eax |= data2;
  pciWriteLong(0x00000000, 0x50, eax);
#endif

  /* TODO: write GUI grant timer (PCI config 0xA3) */

  /* Write CRT/CPU threshold low, CRT/Engine threshold high */
  data = ((ThresholdLow & 0x0f) << 4) | 0x0f;
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x08,data);

  data = (ThresholdLow & 0x10) << 1;
  SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x0F,0xDF,data);

  /* What is this? */
  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x3B,0x09);

  /* Write CRT/CPU threshold high (gap = 3) */
  data = ThresholdLow + 3;
  if(data > 0x0f) data = 0x0f;
  SiS_SetRegANDOR(SiS_Pr->SiS_P3c4,0x09,0x80,data);
}

USHORT
SiS_CalcDelay2(SiS_Private *SiS_Pr, UCHAR *ROMAddr,UCHAR key)
{
  USHORT data,index;
  UCHAR  LatencyFactor[] ={ 97, 88, 86, 79, 77, 00,       /*; 64  bit    BQ=2   */
                            00, 87, 85, 78, 76, 54,       /*; 64  bit    BQ=1   */
                            97, 88, 86, 79, 77, 00,       /*; 128 bit    BQ=2   */
                            00, 79, 77, 70, 68, 48,       /*; 128 bit    BQ=1   */
                            80, 72, 69, 63, 61, 00,       /*; 64  bit    BQ=2   */
                            00, 70, 68, 61, 59, 37,       /*; 64  bit    BQ=1   */
                            86, 77, 75, 68, 66, 00,       /*; 128 bit    BQ=2   */
                            00, 68, 66, 59, 57, 37};      /*; 128 bit    BQ=1   */

  index = (key & 0xE0) >> 5;
  if(key & 0x10) index +=6;
  if(!(key & 0x01)) index += 24;
  data = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x14);
  if(data & 0x0080) index += 12;

  data = LatencyFactor[index];
  return(data);
}
#endif

/* =============== Autodetection ================ */
/*             I N C O M P L E T E                */

BOOLEAN
SiS_GetPanelID(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  const USHORT PanelTypeTable300[16] = {
      0xc101, 0xc117, 0x0121, 0xc135, 0xc142, 0xc152, 0xc162, 0xc072,
      0xc181, 0xc192, 0xc1a1, 0xc1b6, 0xc1c2, 0xc0d2, 0xc1e2, 0xc1f2
  };
  const USHORT PanelTypeTable31030x[16] = {
      0xc102, 0xc112, 0x0122, 0xc132, 0xc142, 0xc152, 0xc169, 0xc179,
      0x0189, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  };
  const USHORT PanelTypeTable310LVDS[16] = {
      0xc111, 0xc122, 0xc133, 0xc144, 0xc155, 0xc166, 0xc177, 0xc188,
      0xc199, 0xc0aa, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
  };
  USHORT tempax,tempbx,tempah,temp;

  if(HwDeviceExtension->jChipType < SIS_315H) {

    tempax = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x18);
    tempbx = tempax & 0x0F;
    if(!(tempax & 0x10)){
      if(SiS_Pr->SiS_IF_DEF_LVDS == 1){
        tempbx = 0;
        temp = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x38);
        if(temp & 0x40) tempbx |= 0x08;
        if(temp & 0x20) tempbx |= 0x02;
        if(temp & 0x01) tempbx |= 0x01;
        temp = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x39);
        if(temp & 0x80) tempbx |= 0x04;
      } else {
        return 0;
      }
    }
    tempbx = PanelTypeTable300[tempbx];
    tempbx |= LCDSync;
    temp = tempbx & 0x00FF;
    SiS_SetReg1(SiS_Pr->SiS_P3d4,0x36,temp);
    temp = (tempbx & 0xFF00) >> 8;
    SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x37,~(LCDSyncBit|LCDRGB18Bit),temp);

  } else {

    tempax = tempah = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1a);
    tempax &= 0x1e;
    tempax >>= 1;
    if(SiS_Pr->SiS_IF_DEF_LVDS == 1) {
       if(tempax == 0) {
           /* TODO: Include HUGE detection routine
	            (Probably not worth bothering)
	    */
           return 0;
       }
       temp = tempax & 0xff;
       tempax--;
       tempbx = PanelTypeTable310LVDS[tempax];
    } else {
       tempbx = PanelTypeTable31030x[tempax];
       temp = tempbx & 0xff;
    }
    SiS_SetReg1(SiS_Pr->SiS_P3d4,0x36,temp);
    tempbx = (tempbx & 0xff00) >> 8;
    temp = tempbx & 0xc1;
    SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x37,~(LCDSyncBit|LCDRGB18Bit),temp);
    if(SiS_Pr->SiS_IF_DEF_LVDS == 0) {
       temp = tempbx & 0x04;
       SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x39,0xfb,temp);
    }

  }
  return 1;
}


#ifdef LINUXBIOS

void
SiS_DetectMonitor(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,USHORT BaseAddr)
{
  UCHAR  DAC_TEST_PARMS[] = {0x0F,0x0F,0x0F};
  UCHAR  DAC_CLR_PARMS[]  = {0x00,0x00,0x00};
  USHORT SR1F;

  SR1F = SiS_GetReg1(SiS_Pr->SiS_P3c4,0x1F);		/* backup DAC pedestal */
  SiS_SetRegOR(SiS_Pr->SiS_P3c4,0x1F,0x04);

  if(SiS_Pr->SiS_IF_DEF_LVDS == 0) {
    if(!(SiS_BridgeIsOn(SiS_Pr, BaseAddr))) {
      SiS_SetReg1(SiS_Pr->SiS_P3d4,0x30,0x41);
    }
  }

  SiSSetMode(SiS_Pr,HwDeviceExtension,0x2E);
  if(HwDeviceExtension->jChipType > SIS_315PRO) {
     /* TW: On 650 only - enable CRT1 */
     SiS_SetRegAND(SiS_Pr->SiS_P3d4,0x63,0xbf);
  }
  SiS_SetReg3(SiS_Pr->SiS_P3c6,0xff);
  SiS_ClearDAC(SiS_Pr, SiS_Pr->SiS_P3c8);
  SiS_LongWait(SiS_Pr);
  SiS_LongWait(SiS_Pr);
  SiS_LongWait(SiS_Pr);
  SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x32,0xDF,0x00);
  if(SiS_TestMonitorType(SiS_Pr, DAC_TEST_PARMS[0],DAC_TEST_PARMS[1],DAC_TEST_PARMS[2])) {
    SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x32,0xDF,0x20);
  } else if(SiS_TestMonitorType(SiS_Pr, DAC_TEST_PARMS[0],DAC_TEST_PARMS[1],DAC_TEST_PARMS[2])) {
    SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x32,0xDF,0x20);
  }
  SiS_TestMonitorType(SiS_Pr, DAC_CLR_PARMS[0],DAC_CLR_PARMS[1],DAC_CLR_PARMS[2]);

  SiS_SetReg1(SiS_Pr->SiS_P3c4,0x1F,SR1F);
}

USHORT
SiS_TestMonitorType(SiS_Private *SiS_Pr, UCHAR R_DAC,UCHAR G_DAC,UCHAR B_DAC)
{
   USHORT temp,tempbx;

   tempbx = R_DAC * 0x4d + G_DAC * 0x97 + B_DAC * 0x1c;
   if((tempbx & 0x00ff) > 0x80) tempbx += 0x100;
   tempbx = (tempbx & 0xFF00) >> 8;
   R_DAC = (UCHAR) tempbx;
   G_DAC = (UCHAR) tempbx;
   B_DAC = (UCHAR) tempbx;

   SiS_SetReg3(SiS_Pr->SiS_P3c8,0x00);
   SiS_SetReg3(SiS_Pr->SiS_P3c9,R_DAC);
   SiS_SetReg3(SiS_Pr->SiS_P3c9,G_DAC);
   SiS_SetReg3(SiS_Pr->SiS_P3c9,B_DAC);
   SiS_LongWait(SiS_Pr);
   temp=SiS_GetReg2(SiS_Pr->SiS_P3c2);
   if(temp & 0x10) return(1);
   else return(0);
}

void
SiS_GetSenseStatus(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension,UCHAR *ROMAddr)
{
  USHORT tempax=0,tempbx,tempcx,temp;
  USHORT P2reg0=0,SenseModeNo=0,OutputSelect=*SiS_Pr->pSiS_OutputSelect;
  USHORT ModeIdIndex,i;
  USHORT BaseAddr = (USHORT)HwDeviceExtension->ulIOAddress;

  if(SiS_Pr->SiS_IF_DEF_LVDS == 1){
    SiS_GetPanelID(SiS_Pr);
    temp=LCDSense;
    temp=temp|SiS_SenseCHTV(SiS_Pr);
    tempbx=~(LCDSense|AVIDEOSense|SVIDEOSense);
    SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x32,tempbx,temp);
  } else {       /* for 301 */
    if(SiS_Pr->SiS_IF_DEF_HiVision==1) {  /* for HiVision */
      tempax=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x38);
      temp=tempax&0x01;
      tempax=SiS_GetReg1(SiS_Pr->SiS_P3c4,0x3A);
      temp=temp|(tempax&0x02);
      SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x32,0xA0,temp);
    } else {
      if(SiS_BridgeIsOn(SiS_Pr, BaseAddr)==0) {    /* TW: Inserted "==0" */
        P2reg0 = SiS_GetReg1(SiS_Pr->SiS_Part2Port,0x00);
        if(!(SiS_BridgeIsEnable(SiS_Pr, BaseAddr,HwDeviceExtension))) {
          SenseModeNo=0x2e;
          temp = SiS_SearchModeID(SiS_Pr, ROMAddr,&SenseModeNo,&ModeIdIndex);
          SiS_Pr->SiS_SetFlag = 0x00;
          SiS_Pr->SiS_ModeType = ModeVGA;
          SiS_Pr->SiS_VBInfo = SetCRT2ToRAMDAC |LoadDACFlag |SetInSlaveMode;
          SiS_SetCRT2Group301(SiS_Pr, BaseAddr,ROMAddr,SenseModeNo,HwDeviceExtension);
          for(i=0;i<20;i++) {
            SiS_LongWait(SiS_Pr);
          }
        }
        SiS_SetReg1(SiS_Pr->SiS_Part2Port,0x00,0x1c);
        tempax=0;
        tempbx=*SiS_Pr->pSiS_RGBSenseData;
	if(SiS_Is301B(SiS_Pr, BaseAddr)){
                tempbx=*SiS_Pr->pSiS_RGBSenseData2;
        }
        tempcx=0x0E08;
        if(SiS_Sense(SiS_Pr, tempbx,tempcx)){
          if(SiS_Sense(SiS_Pr, tempbx,tempcx)){
            tempax=tempax|Monitor2Sense;
          }
        }
        tempbx=*SiS_Pr->pSiS_YCSenseData;
        if(SiS_Is301B(SiS_Pr, BaseAddr)){
               tempbx=*SiS_Pr->pSiS_YCSenseData2;
        }
        tempcx=0x0604;
        if(SiS_Sense(SiS_Pr, tempbx,tempcx)){
          if(SiS_Sense(SiS_Pr,tempbx,tempcx)){
            tempax=tempax|SVIDEOSense;
          }
        }

	if(ROMAddr && SiS_Pr->SiS_UseROM) {
#ifdef SIS300
	   if((HwDeviceExtension->jChipType==SIS_630)||
              (HwDeviceExtension->jChipType==SIS_730)) {
		OutputSelect = ROMAddr[0xfe];
	   }
#endif
#ifdef SIS315H
	   if(HwDeviceExtension->jChipType >= SIS_315H) {
	        OutputSelect = ROMAddr[0xf3];
	   }
#endif
        }
        if(OutputSelect&BoardTVType){
          tempbx=*SiS_Pr->pSiS_VideoSenseData;
          if(SiS_Is301B(SiS_Pr, BaseAddr)){
             tempbx=*SiS_Pr->pSiS_VideoSenseData2;
          }
          tempcx=0x0804;
          if(SiS_Sense(SiS_Pr, tempbx,tempcx)){
            if(SiS_Sense(SiS_Pr, tempbx,tempcx)){
              tempax=tempax|AVIDEOSense;
            }
          }
        } else {
          if(!(tempax&SVIDEOSense)){
            tempbx=*SiS_Pr->pSiS_VideoSenseData;
            if(SiS_Is301B(SiS_Pr, BaseAddr)){
              tempbx=*SiS_Pr->pSiS_VideoSenseData2;
            }
            tempcx=0x0804;
            if(SiS_Sense(SiS_Pr,tempbx,tempcx)){
              if(SiS_Sense(SiS_Pr, tempbx,tempcx)){
                tempax=tempax|AVIDEOSense;
              }
            }
          }
        }
      }

      if(SiS_SenseLCD(SiS_Pr, HwDeviceExtension)){
        tempax=tempax|LCDSense;
      }

      tempbx=0;
      tempcx=0;
      SiS_Sense(SiS_Pr, tempbx,tempcx);

      if(SiS_Pr->SiS_VBType & (VB_SIS30xLV|VB_SIS30xLVX)){   /* TW: prev. 301LV|302LV */
         tempax &= 0x00ef;   /* 301lv to disable CRT2*/
      }
      SiS_SetRegANDOR(SiS_Pr->SiS_P3d4,0x32,~0xDF,tempax);
      SiS_SetReg1(SiS_Pr->SiS_Part2Port,0x00,P2reg0);
      if(!(P2reg0&0x20)) {
        SiS_Pr->SiS_VBInfo = DisableCRT2Display;
        SiS_SetCRT2Group301(SiS_Pr,BaseAddr,ROMAddr,SenseModeNo,HwDeviceExtension);
      }
    }
  }
}

BOOLEAN
SiS_Sense(SiS_Private *SiS_Pr, USHORT tempbx,USHORT tempcx)
{
  USHORT temp,i,tempch;

  temp = tempbx & 0xFF;
  SiS_SetReg1(SiS_Pr->SiS_Part4Port,0x11,temp);
  temp = (tempbx & 0xFF00) >> 8;
  temp |= (tempcx & 0x00FF);
  SiS_SetRegANDOR(SiS_Pr->SiS_Part4Port,0x10,~0x1F,temp);

  for(i=0; i<10; i++) SiS_LongWait(SiS_Pr);

  tempch = (tempcx & 0x7F00) >> 8;
  temp = SiS_GetReg1(SiS_Pr->SiS_Part4Port,0x03);
  temp ^= 0x0E;
  temp &= tempch;
  if(temp>0) return 1;
  else return 0;
}

USHORT
SiS_SenseLCD(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  USHORT temp;

  temp=SiS_GetPanelID(SiS_Pr);
  if(!temp)  temp=SiS_GetLCDDDCInfo(SiS_Pr, HwDeviceExtension);
  return(temp);
}

BOOLEAN
SiS_GetLCDDDCInfo(SiS_Private *SiS_Pr, PSIS_HW_DEVICE_INFO HwDeviceExtension)
{
  USHORT temp;
  /*add lcd sense*/
  if(HwDeviceExtension->ulCRT2LCDType==LCD_UNKNOWN)
    	return 0;
  else{
     	temp=(USHORT)HwDeviceExtension->ulCRT2LCDType;
     	SiS_SetReg1(SiS_Pr->SiS_P3d4,0x36,temp);
  	return 1;
  }
}

USHORT
SiS_SenseCHTV(SiS_Private *SiS_Pr)
{
  USHORT temp,push0e,status;

  status=0;
  push0e = SiS_GetCH700x(SiS_Pr, 0x0e);
  push0e = (push0e << 8) | 0x0e;
  SiS_SetCH700x(SiS_Pr, 0x0b0e);
  SiS_SetCH700x(SiS_Pr, 0x0110);
  SiS_SetCH700x(SiS_Pr, 0x0010);
  temp = SiS_GetCH700x(SiS_Pr, 0x10);
  if(temp & 0x08) status |= SVIDEOSense;
  if(temp & 0x02) status |= AVIDEOSense;
  SiS_SetCH700x(SiS_Pr, push0e);
  return(status);
}
#endif /* LINUXBIOS */

/*  ================ for TC only =================  */

#ifdef TC

int
INT1AReturnCode(union REGS regs)
{
  if (regs.x.cflag)
  {
    /*printf("Error to find pci device!\n"); */
    return 1;
  }

  switch(regs.h.ah)
  {
    case 0: return 0;
            break;
    case 0x81: printf("Function not support\n");
               break;
    case 0x83: printf("bad vendor id\n");
               break;
    case 0x86: printf("device not found\n");
               break;
    case 0x87: printf("bad register number\n");
               break;
    case 0x88: printf("set failed\n");
               break;
    case 0x89: printf("buffer too small");
               break;
  }
  return 1;
}

unsigned
FindPCIIOBase(unsigned index,unsigned deviceid)
{
  union REGS regs;

  regs.h.ah = 0xb1;  /*PCI_FUNCTION_ID */
  regs.h.al = 0x02;  /*FIND_PCI_DEVICE */
  regs.x.cx = deviceid;
  regs.x.dx = 0x1039;
  regs.x.si = index;  /* find n-th device */

  int86(0x1A, &regs, &regs);

  if (INT1AReturnCode(regs)!=0)
    return 0;

  /* regs.h.bh *//* bus number */
  /* regs.h.bl *//* device number */
  regs.h.ah = 0xb1;  /*PCI_FUNCTION_ID */
  regs.h.al = 0x09;  /*READ_CONFIG_WORD */
  regs.x.cx = deviceid;
  regs.x.dx = 0x1039;
  regs.x.di = 0x18;  /* register number */
  int86(0x1A, &regs, &regs);

  if (INT1AReturnCode(regs)!=0)
    return 0;
  return regs.x.cx;
}


void
main(int argc, char *argv[])
{
  SIS_HW_DEVICE_INFO  HwDeviceExtension;
  USHORT temp;
  USHORT ModeNo;

  /*HwDeviceExtension.pjVirtualRomBase =(PUCHAR) MK_FP(0xC000,0); */
  /*HwDeviceExtension.pjVideoMemoryAddress = (PUCHAR)MK_FP(0xA000,0);*/

#ifdef SIS300  
  HwDeviceExtension.ulIOAddress = (FindPCIIOBase(0,0x6300)&0xFF80) + 0x30;
  HwDeviceExtension.jChipType = SIS_630;
#endif

#ifdef SIS315H  
//  HwDeviceExtension.ulIOAddress = (FindPCIIOBase(0,0x5315)&0xFF80) + 0x30;
//  HwDeviceExtension.jChipType = SIS_550;
  HwDeviceExtension.ulIOAddress = (FindPCIIOBase(0,0x325)&0xFF80) + 0x30;
  HwDeviceExtension.jChipType = SIS_315H;
#endif

  HwDeviceExtension.ujVBChipID = VB_CHIP_301;
  strcpy(HwDeviceExtension.szVBIOSVer,"0.84");
  HwDeviceExtension.bSkipDramSizing = FALSE;
  HwDeviceExtension.ulVideoMemorySize = 0;
  if(argc==2) {
    ModeNo=atoi(argv[1]);
  }
  else {
    ModeNo=0x2e;
    /*ModeNo=0x37; */ /* 1024x768x 4bpp */
    /*ModeNo=0x38; *//* 1024x768x 8bpp */
    /*ModeNo=0x4A; *//* 1024x768x 16bpp */
    /*ModeNo=0x47;*/ /* 800x600x 16bpp */
  }
 /* SiSInit(SiS_Pr, &HwDeviceExtension);*/
  SiSSetMode(SiS_Pr, &HwDeviceExtension, ModeNo);
}
#endif /* TC END */

/* ================ LINUX XFREE86 ====================== */

/* Helper functions */

#ifdef LINUX_XF86
USHORT
SiS_CalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode)
{
   SISPtr pSiS = SISPTR(pScrn);
   UShort i = (pSiS->CurrentLayout.bitsPerPixel+7)/8 - 1;
   UShort ModeIndex = 0;

   switch(mode->HDisplay)
   {
     case 320:
          if(mode->VDisplay == 480) {
                ModeIndex = ModeIndex_320x480[i];
	  }
          break;
     case 512:
          if(mode->VDisplay == 384) {
             ModeIndex = ModeIndex_512x384[i];
	  }
          break;
     case 640:
          if(mode->VDisplay == 480) {
             ModeIndex = ModeIndex_640x480[i];
	  }
          break;
     case 720:
          if(mode->VDisplay == 480) {
                ModeIndex = ModeIndex_720x480[i];
          } else if(mode->VDisplay == 576) {
                ModeIndex = ModeIndex_720x576[i];
          }
          break;
     case 800:
	  if(mode->VDisplay == 600) {
             ModeIndex = ModeIndex_800x600[i];
	  } else if(pSiS->VGAEngine == SIS_315_VGA) {
	     if(mode->VDisplay == 480) {
	           ModeIndex = ModeIndex_800x480[i];
             }
	  }
          break;
     case 1024:
          if(mode->VDisplay == 768) {
	        ModeIndex = ModeIndex_1024x768[i];
	  } else if(pSiS->VGAEngine == SIS_315_VGA) {
	     if(mode->VDisplay == 576) {
	        ModeIndex = ModeIndex_1024x576[i];
             }
	  } else if(pSiS->VGAEngine == SIS_300_VGA) {
	     if(mode->VDisplay == 600) {
	        ModeIndex = ModeIndex_1024x600[i];
             }
	  }
          break;
     case 1152:
          if(pSiS->VGAEngine == SIS_300_VGA) {
	     if(mode->VDisplay == 768) {
	        ModeIndex = ModeIndex_1152x768[i];
             }
	  }
	  break;
     case 1280:
          if(mode->VDisplay == 960) {
             if(pSiS->VGAEngine == SIS_300_VGA) {
	        ModeIndex = ModeIndex_300_1280x960[i];
             } else {
                ModeIndex = ModeIndex_310_1280x960[i];
             }
	  } else if (mode->VDisplay == 1024) {
	     ModeIndex = ModeIndex_1280x1024[i];
	  } else if(pSiS->VGAEngine == SIS_315_VGA) {
	     if (mode->VDisplay == 768) {
	        ModeIndex = ModeIndex_1280x768[i];
	     } else if (mode->VDisplay == 720) {
	        ModeIndex = ModeIndex_1280x720[i];
             }
	  }
          break;
     case 1400:
          if(pSiS->VGAEngine == SIS_315_VGA) {
	     if(mode->VDisplay == 1050) {
	        ModeIndex = ModeIndex_1400x1050[i];
             }
	  }
          break;
     case 1600:
          if(mode->VDisplay == 1200) {
             ModeIndex = ModeIndex_1600x1200[i];
	  }
          break;
     case 1920:
          if(mode->VDisplay == 1440) {
             ModeIndex = ModeIndex_1920x1440[i];
	  }
          break;
     case 2048:
          if(pSiS->VGAEngine == SIS_315_VGA) {
	     if(mode->VDisplay == 1536) {
	         ModeIndex = ModeIndex_2048x1536[i];
             }
	  }
          break;
   }

   return(ModeIndex);
}

USHORT
SiS_CheckCalcModeIndex(ScrnInfoPtr pScrn, DisplayModePtr mode, int VBFlags)
{
   SISPtr pSiS = SISPTR(pScrn);
   UShort i = (pSiS->CurrentLayout.bitsPerPixel+7)/8 - 1;    
   UShort ModeIndex = 0;

   if(VBFlags & CRT2_LCD) {

      if( (mode->HDisplay <= pSiS->LCDwidth) &&
          (mode->VDisplay <= pSiS->LCDheight) ) {

        if(VBFlags & VB_LVDS) {        		/* LCD on LVDS */

          switch(mode->HDisplay)
  	  {
	  case 512:
		if(mode->VDisplay == 384) {
		   ModeIndex = ModeIndex_512x384[i];
		}
		break;
	  case 640:
		if(mode->VDisplay == 480) {
		   ModeIndex = ModeIndex_640x480[i];
		}
		break;
	  case 800:
		if(mode->VDisplay == 600) {
		   ModeIndex = ModeIndex_800x600[i];
		}
		break;
	  case 1024:
		if(mode->VDisplay == 768) {
		   ModeIndex = ModeIndex_1024x768[i];
		} else if(pSiS->VGAEngine == SIS_300_VGA) {
		   if(mode->VDisplay == 600) {
		      ModeIndex = ModeIndex_1024x600[i];
		   }
		}
		break;
	  case 1152:
		if(pSiS->VGAEngine == SIS_300_VGA) {
		   if(mode->VDisplay == 768) {
			ModeIndex = ModeIndex_1152x768[i];
		   }
		}
		break;
	  case 1280:
		if(mode->VDisplay == 1024) {
		   ModeIndex = ModeIndex_1280x1024[i];
		} else if(pSiS->VGAEngine == SIS_315_VGA) {
		   if(mode->VDisplay == 768) {
		      ModeIndex = ModeIndex_1280x768[i];
		   }
		}
		break;
	  case 1400:
	        if(mode->VDisplay == 1050) {
		   if(pSiS->VGAEngine == SIS_315_VGA) {
		      ModeIndex = ModeIndex_1400x1050[i];
		   }
		}
		break;
          }

        } else {                       	 	/* LCD on 301(B) */

          switch(mode->HDisplay)
	  {
	  case 512:
		if(mode->VDisplay == 384) {
		   ModeIndex = ModeIndex_512x384[i];
		}
		break;
	  case 640:
		if(mode->VDisplay == 480) {
		   ModeIndex = ModeIndex_640x480[i];
		}
		break;
	  case 800:
		if(mode->VDisplay == 600) {
		   ModeIndex = ModeIndex_800x600[i];
		}
		break;
	  case 1024:
		if(mode->VDisplay == 768) {
		   ModeIndex = ModeIndex_1024x768[i];
		} /* else if(pSiS->VGAEngine == SIS_300_VGA) {  --  not supported on 301(B) --
		   if(mode->VDisplay == 600) {
			ModeIndex = ModeIndex_1024x600[i];
		   }
		} */
		break;
	  case 1152:  /* not supported on 301(B) */
		break;
	  case 1280:
		if(mode->VDisplay == 960) {
		   if(pSiS->VGAEngine == SIS_300_VGA) {
		      ModeIndex = ModeIndex_300_1280x960[i];
		   } else {
		      ModeIndex = ModeIndex_310_1280x960[i];
		   }
                } else if (mode->VDisplay == 1024) {
	             ModeIndex = ModeIndex_1280x1024[i];
	        }
	  case 1600:
		if(mode->VDisplay == 1200) {
		   ModeIndex = ModeIndex_1600x1200[i];
		}
		break;
	  }

        }

      }

   } else if(VBFlags & CRT2_TV) {

      if(VBFlags & VB_CHRONTEL) {		/* TV on Chrontel */

        switch(mode->HDisplay)
	{
      	case 512:
		if(mode->VDisplay == 384) {
		   ModeIndex = ModeIndex_512x384[i];
		}
		break;
	case 640:
		if(mode->VDisplay == 480) {
		   ModeIndex = ModeIndex_640x480[i];
		}
		break;
	case 800:
		if(mode->VDisplay == 600) {
		   ModeIndex = ModeIndex_800x600[i];
		}
		break;
	case 1024:
		if(mode->VDisplay == 768) {
		   if(pSiS->VGAEngine == SIS_315_VGA) {
		      ModeIndex = ModeIndex_1024x768[i];
		   }
		}
		break;
        }

      } else {				    /* TV on 301(B) */

        switch(mode->HDisplay)
	{
      	case 512:
		if(mode->VDisplay == 384) {
		   ModeIndex = ModeIndex_512x384[i];
		}
		break;
	case 640:
		if(mode->VDisplay == 480) {
		   ModeIndex = ModeIndex_640x480[i];
		}
		break;
	case 720:
                if(mode->VDisplay == 480) {
                   ModeIndex = ModeIndex_720x480[i];
                } else if(mode->VDisplay == 576) {
                   ModeIndex = ModeIndex_720x576[i];
                }
                break;
	case 800:
		if(mode->VDisplay == 600) {
		   ModeIndex = ModeIndex_800x600[i];
		}
		break;
	case 1024:
		if(mode->VDisplay == 768) {
		   if(VBFlags & (VB_301B|VB_302B|VB_30xLV|VB_30xLVX)) {
		      ModeIndex = ModeIndex_1024x768[i];
		   }
		}
		break;
        }

      }

   } else if(VBFlags & CRT2_VGA) {		/* CRT2 is VGA2 */

	switch(mode->HDisplay)
	{
	case 512:
		if(mode->VDisplay == 384) {
		    ModeIndex = ModeIndex_512x384[i];
		}
		break;
	case 640:
		if(mode->VDisplay == 480) {
		   ModeIndex = ModeIndex_640x480[i];
		}
		break;
	case 800:
		if(mode->VDisplay == 600) {
		   ModeIndex = ModeIndex_800x600[i];
		} else if(pSiS->VGAEngine == SIS_315_VGA) {
		   if(mode->VDisplay == 480) {
			ModeIndex = ModeIndex_800x480[i];
		   }
		}
		break;
	case 1024:
		if(mode->VDisplay == 768) {
			ModeIndex = ModeIndex_1024x768[i];
		} else if(pSiS->VGAEngine == SIS_315_VGA) {
		   if(mode->VDisplay == 576) {
			ModeIndex = ModeIndex_1024x576[i];
		   }
		}
		break;
	case 1152:
		if(pSiS->VGAEngine == SIS_300_VGA) {
		   if(mode->VDisplay == 768) {
			ModeIndex = ModeIndex_1152x768[i];
		   }
		}
		break;
	case 1280:
		if (mode->VDisplay == 1024) {
		   ModeIndex = ModeIndex_1280x1024[i];
		} else if(pSiS->VGAEngine == SIS_315_VGA) {
		   if (mode->VDisplay == 768) {
			ModeIndex = ModeIndex_1280x768[i];
		   } else if (mode->VDisplay == 720) {
			ModeIndex = ModeIndex_1280x720[i];
		   }
		}
		break;
	case 1400:
		if(pSiS->VGAEngine == SIS_315_VGA) {
		   ModeIndex = ModeIndex_1400x1050[i];
		}
		break;
	}

   } else {				/* CRT1 only, no CRT2 */

       ModeIndex = SiS_CalcModeIndex(pScrn, mode);

   }

   return(ModeIndex);
}

#define MODEID_OFF 0x449

unsigned char
SiS_GetSetModeID(ScrnInfoPtr pScrn, unsigned char id)
{
    return(SiS_GetSetMMIOReg(pScrn, MODEID_OFF, id));
}

unsigned char
SiS_GetSetMMIOReg(ScrnInfoPtr pScrn, USHORT offset, unsigned char value)
{
    unsigned char ret;
    unsigned char *base;
    SISPtr pSiS = SISPTR(pScrn);
    BOOLEAN mapped;

    if(pSiS->IOBase) {
    	base = (unsigned char *)pSiS->IOBase;
	mapped = FALSE;
    } else {
        base = xf86MapVidMem(pScrn->scrnIndex, VIDMEM_MMIO, 0, 0x2000);
	if(!base) {
	     xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
	          "(init.c: Could not MMIO area!)\n");
	     return 0;
	}
	mapped = TRUE;
    }

    ret = *(base + offset);

    /* value != 0xff means: set register */
    if (value != 0xff)
	*(base + offset) = value;

    if(mapped) xf86UnMapVidMem(pScrn->scrnIndex, base, 0x2000);

    return ret;
}

#endif
