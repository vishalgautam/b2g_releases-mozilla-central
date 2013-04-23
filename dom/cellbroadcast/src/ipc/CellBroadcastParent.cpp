/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "CellBroadcastParent.h"

using namespace mozilla::dom::cellbroadcast;

NS_IMPL_ISUPPORTS1(CellBroadcastParent, nsICellBroadcastListener)

void
CellBroadcastParent::ActorDestroy(ActorDestroyReason aWhy)
{
  mActorDestroyed = true;

  nsCOMPtr<nsICellBroadcastProvider> provider =
    do_GetService(CELLBROADCAST_SERVICE_CONTRACTID);
  provider->Unregister(this);
}

NS_IMETHODIMP
CellBroadcastParent::NotifyMessageReceived(nsIDOMMozCellBroadcastMessage* aMessage)
{
  // The child process could die before this asynchronous notification, in which
  // case ActorDestroy() was called and mActorDestroyed is set to true. Return
  // an error here to avoid sending a message to the dead process.
  NS_ENSURE_TRUE(!mActorDestroyed, NS_ERROR_FAILURE);

  // FIXME: return SendNotifyMessageReceived(aMessage->GetData()) ? NS_OK : NS_ERROR_FAILURE;
  return NS_OK;
}

