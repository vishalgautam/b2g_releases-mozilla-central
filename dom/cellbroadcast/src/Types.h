/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_cellbroadcast_Types_h
#define mozilla_dom_cellbroadcast_Types_h

#include "IPCMessageUtils.h"

namespace mozilla {
namespace dom {
namespace cellbroadcast {

enum GeographicalScope
{
  eGeographicalScope_CellImmediate = 0,
  eGeographicalScope_PLMN,
  eGeographicalScope_LocationArea,
  eGeographicalScope_Cell,
  // This state should stay at the end.
  eGeographicalScope_EndGuard
};

enum MessageClass
{
  eMessageClass_Normal = 0,
  eMessageClass_Class0,
  eMessageClass_Class1,
  eMessageClass_Class2,
  eMessageClass_Class3,
  eMessageClass_User1,
  eMessageClass_User2,
  // This state should stay at the end.
  eMessageClass_EndGuard
};

enum WarningType
{
  eWarningType_EarthQuake = 0,
  eWarningType_Tsunami,
  eWarningType_EarthQuake_Tsunami,
  eWarningType_Test,
  eWarningType_Other,
  // This state should stay at the end.
  eWarningType_EndGuard
};

} // namespace cellbroadcast
} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_cellbroadcast_Types_h
