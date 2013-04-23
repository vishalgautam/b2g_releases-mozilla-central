/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_cellbroadcast_CellBroadcastIPCService_h
#define mozilla_dom_cellbroadcast_CellBroadcastIPCService_h

#include "mozilla/Attributes.h"
#include "nsCOMPtr.h"
#include "nsTArray.h"
#include "nsICellBroadcastProvider.h"

namespace mozilla {
namespace dom {
namespace cellbroadcast {

class PCellBroadcastChild;

class CellBroadcastIPCService MOZ_FINAL : public nsICellBroadcastProvider
                                        , public nsICellBroadcastListener
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICELLBROADCASTPROVIDER
  NS_DECL_NSICELLBROADCASTLISTENER

  CellBroadcastIPCService() {}

private:
  virtual ~CellBroadcastIPCService();

  void ReleaseIPCChild();

  PCellBroadcastChild* mChild;
  nsTArray< nsCOMPtr<nsICellBroadcastListener> > mListeners;
};

} // namespace cellbroadcast
} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_cellbroadcast_CellBroadcastIPCService_h
