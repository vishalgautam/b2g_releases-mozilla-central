/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_cellbroadcast_ServiceFactory_h
#define mozilla_dom_cellbroadcast_ServiceFactory_h

#include "nsCOMPtr.h"

class nsICellBroadcastProvider;

namespace mozilla {
namespace dom {
namespace cellbroadcast {

class ServiceFactory
{
public:
  static already_AddRefed<nsICellBroadcastProvider>
  CreateCellBroadcastService();
};

} // namespace cellbroadcast
} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_cellbroadcast_ServiceFactory_h

