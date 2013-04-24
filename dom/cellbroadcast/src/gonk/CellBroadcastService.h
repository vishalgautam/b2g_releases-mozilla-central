/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_cellbroadcast_CellBroadcastService_h
#define mozilla_dom_cellbroadcast_CellBroadcastService_h

#include "mozilla/Attributes.h"
#include "nsICellBroadcastProvider.h"
#include "nsCOMPtr.h"
#include "nsTArray.h"

namespace mozilla {
namespace dom {
namespace cellbroadcast {

class PSmsChild;

class CellBroadcastService MOZ_FINAL : public nsICellBroadcastProvider
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICELLBROADCASTPROVIDER

private:
  virtual ~CellBroadcastService() {}

  nsTArray< nsCOMPtr<nsICellBroadcastListener> > mListeners;
};

} // namespace cellbroadcast
} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_cellbroadcast_CellBroadcastService_h
