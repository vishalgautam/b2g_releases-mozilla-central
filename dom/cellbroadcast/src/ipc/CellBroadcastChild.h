/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef mozilla_dom_cellbroadcast_CellBroadcastChild_h
#define mozilla_dom_cellbroadcast_CellBroadcastChild_h

#include "mozilla/Attributes.h"
#include "mozilla/dom/cellbroadcast/PCellBroadcastChild.h"
#include "nsICellBroadcastProvider.h"

class nsICellBroadcastListener;

namespace mozilla {
namespace dom {
namespace cellbroadcast {

class CellBroadcastChild MOZ_FINAL : public PCellBroadcastChild
{
public:
  CellBroadcastChild(nsICellBroadcastListener* aListener)
    : mListener(aListener)
  {}

private:
  virtual ~CellBroadcastChild() {}

  virtual bool
  RecvNotifyMessageReceived(const MessageData& aMessageData) MOZ_OVERRIDE;

private:
  nsCOMPtr<nsICellBroadcastListener> mListener;
};

} // namespace cellbroadcast
} // namespace dom
} // namespace mozilla

#endif // mozilla_dom_cellbroadcast_CellBroadcastChild_h

