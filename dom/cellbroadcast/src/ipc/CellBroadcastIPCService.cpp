/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/dom/ContentChild.h"
#include "CellBroadcastChild.h"
#include "CellBroadcastIPCService.h"

using namespace mozilla::dom::cellbroadcast;

NS_IMPL_ISUPPORTS2(CellBroadcastIPCService,
                   nsICellBroadcastProvider,
                   nsICellBroadcastListener);

CellBroadcastIPCService::~CellBroadcastIPCService()
{
  if (mChild) {
    ReleaseIPCChild();
  }
}

void
CellBroadcastIPCService::ReleaseIPCChild()
{
  PCellBroadcastChild::Send__delete__(mChild);
  mChild = nullptr;
}

NS_IMETHODIMP
CellBroadcastIPCService::Register(nsICellBroadcastListener* aListener)
{
  NS_ENSURE_STATE(!mListeners.Contains(aListener));
  NS_ENSURE_TRUE(mListeners.AppendElement(aListener), NS_ERROR_FAILURE);

  if (!mChild) {
    mChild = new CellBroadcastChild(this);
    ContentChild::GetSingleton()->SendPCellBroadcastConstructor(mChild);
  }

  return NS_OK;
}

NS_IMETHODIMP
CellBroadcastIPCService::Unregister(nsICellBroadcastListener* aListener)
{
  NS_ENSURE_TRUE(mListeners.RemoveElement(aListener), NS_ERROR_FAILURE);

  if (mListeners.IsEmpty()) {
    ReleaseIPCChild();
  }

  return NS_OK;
}

NS_IMETHODIMP
CellBroadcastIPCService::NotifyMessageReceived(nsIDOMMozCellBroadcastMessage* aMessage)
{
  const uint32_t length = mListeners.Length();
  for (uint32_t index = 0; index < length; index++) {
    mListeners[index]->NotifyMessageReceived(aMessage);
  }
  return NS_OK;
}
