/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_cellbroadcast_CellBroadcastParent_h
#define mozilla_dom_cellbroadcast_CellBroadcastParent_h

#include "mozilla/Attributes.h"
#include "mozilla/dom/cellbroadcast/PCellBroadcastParent.h"
#include "nsICellBroadcastProvider.h"

namespace mozilla {
namespace dom {
namespace cellbroadcast {

class CellBroadcastParent MOZ_FINAL : public PCellBroadcastParent
                                    , public nsICellBroadcastListener
{
private:
  bool mActorDestroyed;

public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSICELLBROADCASTLISTENER

  CellBroadcastParent()
    : mActorDestroyed(false) {}

protected:
  virtual ~CellBroadcastParent() {}

  virtual void
  ActorDestroy(ActorDestroyReason aWhy) MOZ_OVERRIDE;
};

} // namespace cellbroadcast
} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_cellbroadcast_CellBroadcastParent_h
